#pragma once
#include "engine/quiescence.hpp"

namespace CastleMate::engine {
constexpr auto infinity_v = 1'000'000;

struct NegamaxResult {
	int eval{};
	Move move{};
};

inline auto negamax(Position& pos, int depth, int alpha, int beta, SearchContext& ctx) -> NegamaxResult {
	if (ctx.check_time()) { return {}; }

	if (depth == 0) { return {.eval = quiescence(pos, alpha, beta, ctx)}; }

	auto legal = legal_moves(pos);
	if (legal.empty()) {
		if (in_check(pos, pos.turn == Color::White)) { return {.eval = -infinity_v}; }
		return {};
	}

	std::ranges::sort(legal, std::greater{}, [&](Move const& m) {
		return move_score(pos, m);
	});

	auto best = NegamaxResult{.eval = -infinity_v, .move = legal.front()};
	for (auto move : legal) {
		auto undo = make_move(pos, move);
		auto eval = -negamax(pos, depth - 1, -beta, -alpha, ctx).eval;
		unmake_move(pos, undo);

		if (ctx.timed_out) { break; }

		if (eval > best.eval) { best = {.eval = eval, .move = move}; }

		alpha = std::max(alpha, eval);
		if (alpha >= beta) {
			break; // Bad branch
		}
	}

	return best;
}
} // namespace CastleMate::engine