#pragma once
#include "castlemate/core/movegen.hpp"
#include "engine/evalutation.hpp"
#include "engine/move_score.hpp"
#include <algorithm>

namespace CastleMate::engine {
constexpr auto infinity_v = 1'000'000;

struct NegamaxResult {
	int eval{};
	Move move{};
};

inline auto negamax(Position& pos, int depth, int alpha, int beta) -> NegamaxResult {
	if (depth == 0) {
		auto eval = tapered_eval(pos);
		return {.eval = pos.turn == Color::White ? eval : -eval};
	}

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

		auto eval = -negamax(pos, depth - 1, -beta, -alpha).eval;

		unmake_move(pos, undo);

		if (eval > best.eval) { best = {.eval = eval, .move = move}; }

		alpha = std::max(alpha, eval);
		if (alpha >= beta) {
			break; // Bad branch
		}
	}

	return best;
}
} // namespace CastleMate::engine