#include "engine/engine.hpp"
#include "castlemate/core/movegen.hpp"
#include "castlemate/utils/algebraic.hpp"
#include <print>
#include <sstream>
#include <string>

namespace CastleMate::engine {
namespace {
constexpr auto start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

auto rng() -> std::mt19937& {
	static std::mt19937 rng{std::random_device{}()};
	return rng;
}

auto cmd_uci(std::ostream& out) {
	std::println(out, "id name CastleMateEngine");
	std::println(out, "id author StanPlayzz");
	std::println(out, "uciok");
}

auto parse_position(std::istringstream& stream) {
	auto pos = Position{};
	auto token = std::string{};
	stream >> token;

	if (token == "startpos") {
		pos = Position::from_fen(start_fen);
		stream >> token;
	} else if (token == "fen") {
		auto fen = std::string{};
		while (stream >> token && token != "moves") { fen += token + " "; }
		pos = Position::from_fen(fen);
	}

	if (token == "moves") {
		std::string move{};
		while (stream >> move) {
			for (auto m : legal_moves(pos)) {
				if (to_uci(m) == move) {
					make_move(pos, m);
					break;
				}
			}
		}
	}

	return pos;
}

auto cmd_go(std::ostream& out, Position& pos) {
	auto moves = legal_moves(pos);
	if (moves.empty()) {
		std::println(out, "bestmove 0000");
		return;
	}

	auto& move = moves.at(rng()() % moves.size());

	std::println(out, "bestmove {}", to_uci(move));
}
} // namespace

void Engine::run() {
	auto line = std::string{};
	while (std::getline(m_in, line)) {
		if (line == "quit") { break; }
		handle_command(line);
	}
}

void Engine::handle_command(std::string const& line) {
	auto stream = std::istringstream{line};
	auto cmd = std::string{};
	stream >> cmd;

	if (cmd == "uci") {
		cmd_uci(m_out);
	} else if (cmd == "isready") {
		std::println(m_out, "readyok");
	} else if (cmd == "ucinewgame") {
		new_game();
	} else if (cmd == "position") {
		m_position = parse_position(stream);
	} else if (cmd == "go") {
		cmd_go(m_out, m_position);
	}

	m_out.flush();
}

void Engine::new_game() { m_position = Position::from_fen(start_fen); }
} // namespace CastleMate::engine