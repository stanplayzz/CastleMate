#include "castlemate/engine/native_engine.hpp"
#include "castlemate/utils/algebraic.hpp"

namespace CastleMate {
void NativeEngine::new_game() { send("ucinewgame"); }

void NativeEngine::set_position(Position const& position) {
	m_position = position;
	send("position fen " + position.to_fen());
}

void NativeEngine::go(engine::SearchParams params) {
	auto cmd = std::string{"go"};
	if (params.movetime) { cmd += " movetime " + std::to_string(params.movetime->count()); }
	if (params.depth) { cmd += " depth " + std::to_string(*params.depth); }
	send(cmd);
}

void NativeEngine::stop() { send("stop"); }

void NativeEngine::send(std::string const& cmd) {
	m_engine.handle_command(cmd);

	auto output = m_out.str();
	m_out.str("");

	auto lines = std::istringstream{output};
	auto line = std::string{};
	while (std::getline(lines, line)) { handle_line(line); }
}

void NativeEngine::handle_line(std::string const& line) {
	auto stream = std::istringstream{line};
	auto token = std::string{};
	stream >> token;

	if (token == "bestmove" && m_on_best_move) {
		auto mv_str = std::string{};
		stream >> mv_str;
		if (auto mv = from_uci(mv_str, m_position); mv) { m_on_best_move(*mv); }
	} else if (token == "info" && m_on_info) {
		m_on_info(line);
	}
}
} // namespace CastleMate