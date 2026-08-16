#pragma once
#include "castlemate/core/movegen.hpp"
#include "castlemate/utils/algebraic.hpp"
#include <print>

namespace CastleMate::engine {
constexpr auto perft(Position& pos, int depth) -> std::uint64_t {
	if (depth == 0) { return 1; }

	auto nodes = std::uint64_t{};

	for (auto move : legal_moves(pos)) {
		auto undo = make_move(pos, move);
		nodes += perft(pos, depth - 1);
		unmake_move(pos, undo);
	}

	return nodes;
}

inline auto perft_divide(Position& pos, int depth) {
	for (auto move : legal_moves(pos)) {
		auto undo = make_move(pos, move);
		auto nodes = perft(pos, depth - 1);
		unmake_move(pos, undo);

		auto algebraic = to_algebraic(move, pos);
		std::println("{}: {}", algebraic, nodes);
	}
}
} // namespace CastleMate::engine