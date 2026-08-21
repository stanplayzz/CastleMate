#include "castlemate/engine/uci_engine.hpp"
#include "castlemate/utils/algebraic.hpp"

namespace CastleMate {
UciEngine::UciEngine(std::filesystem::path const& engine_binary) {
	auto ec = m_process.start(std::vector<std::string>{engine_binary.string()});
	if (ec) { throw std::runtime_error{"Failed to start engine: " + ec.message()}; }

	send("uci");
	send("isready");

	m_reader_thread = std::jthread{[this](std::stop_token const& token) {
		reader_loop(token);
	}};
}

UciEngine::~UciEngine() {
	send("quit");
	m_process.wait(reproc::milliseconds(1000));
}

void UciEngine::new_game() { send("ucinewgame"); }

void UciEngine::set_position(Position const& position) {
	{
		std::lock_guard lock{m_position_mutex};
		m_position = position;
	}
	auto cmd = std::string{"position fen "} + position.to_fen();
	send(cmd);
}

void UciEngine::go() { send("go"); }

void UciEngine::stop() { send("stop"); }

void UciEngine::send(std::string const& cmd) {
	auto line = cmd + "\n";
	auto const* data = reinterpret_cast<std::uint8_t const*>(line.data()); // NOLINT
	m_process.write(data, line.size());
}

void UciEngine::reader_loop(std::stop_token const& token) {
	auto buffer = std::string{};
	auto chunk = std::array<std::uint8_t, 4096>{};

	while (!token.stop_requested()) {
		auto [bytes, ec] = m_process.read(reproc::stream::out, chunk.data(), sizeof(chunk));
		if (ec) { break; }

		buffer.append(reinterpret_cast<char*>(chunk.data()), bytes); // NOLINT

		auto i = std::size_t{};
		while ((i = buffer.find('\n')) != std::string::npos) {
			auto line = buffer.substr(0, i);
			buffer.erase(0, i + 1);
			if (!line.empty() && line.back() == '\r') { line.pop_back(); }
			handle_line(line);
		}
	}
}

void UciEngine::handle_line(std::string const& line) {
	auto stream = std::istringstream{line};
	auto token = std::string{};
	stream >> token;

	if (token == "bestmove") {
		auto str = std::string{};
		stream >> str;

		std::optional<Move> move;
		{
			std::lock_guard lock{m_position_mutex};
			move = from_uci(str, m_position);
		}

		if (move && m_on_best_move) { m_on_best_move(*move); }
	} else if (token == "info") {
		if (m_on_info) { m_on_info(line); }
	}
}
} // namespace CastleMate