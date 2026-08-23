#pragma once
#include "engine/negamax.hpp"

namespace CastleMate::engine {
inline auto find_best_move(Position& pos, std::chrono::milliseconds movetime) -> Move {
	auto ctx = SearchContext{.deadline = std::chrono::steady_clock::now() + movetime};

	auto best_move = legal_moves(pos).front();

	for (auto depth = 1; depth <= 64; ++depth) {
		if (ctx.check_time()) { break; }
		auto res = negamax(pos, depth, -infinity_v, infinity_v, ctx);
		if (ctx.timed_out) { break; } // Corrupt result
		best_move = res.move;
	}

	return best_move;
}
} // namespace CastleMate::engine