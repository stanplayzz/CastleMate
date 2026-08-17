#include "engine/engine.hpp"
#include "castlemate/core/movegen.hpp"
#include "castlemate/utils/algebraic.hpp"
#include <iostream>
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

constexpr auto cmd_uci() {
	std::println("id name CastleMateEngine");
	std::println("id author StanPlayzz");
	std::println("uciok");
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

constexpr auto cmd_go(Position& pos) {
	auto moves = legal_moves(pos);
	if (moves.empty()) {
		std::println("bestmove 0000");
		return;
	}

	auto& move = moves.at(rng()() % moves.size());

	std::println("bestmove {}", to_uci(move));
}
} // namespace

Engine::Engine() {
	auto line = std::string{};
	while (std::getline(std::cin, line)) {
		auto stream = std::istringstream{line};
		auto cmd = std::string{};
		stream >> cmd;

		if (cmd == "uci") {
			cmd_uci();
		} else if (cmd == "isready") {
			std::println("readyok");
		} else if (cmd == "ucinewgame") {
			new_game();
		} else if (cmd == "position") {
			m_position = parse_position(stream);
		} else if (cmd == "go") {
			cmd_go(m_position);
		} else if (cmd == "quit") {
			break;
		}

		std::cout.flush();
	}
}

void Engine::new_game() { m_position = Position::from_fen(start_fen); }
} // namespace CastleMate::engine