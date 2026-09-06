#pragma once
#include "castlemate/utils/algebraic.hpp"
#include "engine/negamax.hpp"
#include <print>

namespace CastleMate::engine {
inline auto find_best_move(Position& pos, std::chrono::milliseconds movetime, SearchContext& ctx) -> Move {
	auto start = std::chrono::steady_clock::now();
	ctx.reset(start + movetime);

	auto best_move = legal_moves(pos).front();

	for (auto depth = 1; depth <= 64; ++depth) {
		if (ctx.check_time()) { break; }
		auto res = negamax(pos, depth, -infinity_v, infinity_v, ctx);
		if (ctx.timed_out) { break; } // Corrupt result
		best_move = res.move;

		auto elapsed = std::chrono::steady_clock::now() - start;
		if (std::abs(res.eval) >= mate_value_v - 1000) {
			auto const ply_to_mate = mate_value_v - std::abs(res.eval);
			auto const moves_to_mate = (ply_to_mate + 1) / 2;
			auto const signed_mate = res.eval > 0 ? moves_to_mate : -moves_to_mate;
			std::println("info depth {} score mate {} time {} pv {}", depth, signed_mate, elapsed.count(),
						 to_uci(best_move));
		} else {
			std::println("info depth {} score cp {} time {} pv {}", depth, res.eval, elapsed.count(),
						 to_uci(best_move));
		}
	}

	return best_move;
}
} // namespace CastleMate::engine