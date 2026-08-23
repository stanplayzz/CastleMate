#pragma once
#include "castlemate/core/movegen.hpp"
#include "engine/move_score.hpp"
#include "engine/search_context.hpp"
#include <algorithm>

namespace CastleMate::engine {
inline auto quiescence(Position& pos, int alpha, int beta, SearchContext& ctx) {
	if (ctx.check_time()) { return 0; }

	auto pat = pos.turn == Color::White ? tapered_eval(pos) : -tapered_eval(pos);

	if (pat >= beta) { return beta; }
	alpha = std::max(pat, alpha);

	auto legal = legal_moves(pos);

	auto captures = std::vector<Move>{};
	for (auto const& m : legal) {
		if (get_bit(pos.occ, m.to)) { captures.push_back(m); } // bug: doesn't contain en passant
	}

	std::ranges::sort(captures, std::greater{}, [&](Move const& m) {
		return move_score(pos, m);
	});

	for (auto const& move : captures) {
		auto undo = make_move(pos, move);
		auto eval = -quiescence(pos, -beta, -alpha, ctx);
		unmake_move(pos, undo);

		if (ctx.timed_out) { break; }

		if (eval >= beta) { return beta; }
		alpha = std::max(eval, alpha);
	}

	return alpha;
}
} // namespace CastleMate::engine