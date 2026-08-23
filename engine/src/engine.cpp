#include "engine/engine.hpp"
#include "castlemate/utils/algebraic.hpp"
#include "engine/best_move.hpp"
#include "engine/movetime.hpp"
#include <print>
#include <sstream>
#include <string>

namespace CastleMate::engine {
namespace {
constexpr auto start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

auto parse_go_movetime(std::istringstream& stream, Position const& pos) -> std::chrono::milliseconds {
	auto movetime = std::optional<std::chrono::milliseconds>{};
	auto wtime = std::optional<int>{};
	auto btime = std::optional<int>{};
	auto winc = 0;
	auto binc = 0;

	auto token = std::string{};
	while (stream >> token) {
		auto next_int = [&]() -> int {
			auto s = std::string{};
			stream >> s;
			return std::stoi(s);
		};

		if (token == "movetime") {
			movetime = std::chrono::milliseconds{next_int()};
		} else if (token == "wtime") {
			wtime = next_int();
		} else if (token == "btime") {
			btime = next_int();
		} else if (token == "winc") {
			winc = next_int();
		} else if (token == "binc") {
			binc = next_int();
		}
	}

	if (movetime) { return *movetime; }

	auto const is_white = pos.turn == Color::White;
	auto const remaining = is_white ? wtime : btime;
	auto const inc = is_white ? winc : binc;

	return remaining ? compute_movetime(*remaining, inc) : std::chrono::milliseconds{1000};
}

auto cmd_go(std::ostream& out, Position& pos, std::chrono::milliseconds movetime) {
	auto moves = legal_moves(pos);

	if (moves.empty()) {
		std::println(out, "{}", "bestmove 0000");
		return;
	}

	auto move = find_best_move(pos, movetime);
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
		cmd_go(m_out, m_position, parse_go_movetime(stream, m_position));
	}

	m_out.flush();
}

void Engine::new_game() { m_position = Position::from_fen(start_fen); }
} // namespace CastleMate::engine