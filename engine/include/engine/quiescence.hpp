#pragma once
#include "castlemate/core/movegen.hpp"
#include "castlemate/core/repetition.hpp"
#include "engine/move_score.hpp"
#include "engine/search_context.hpp"
#include <algorithm>

namespace CastleMate::engine {
inline auto quiescence(Position& pos, int alpha, int beta, SearchContext& ctx) {
	if (ctx.check_time() || is_draw(pos)) { return 0; }

	auto const check = in_check(pos, pos.turn == Color::White);

	auto moves = legal_moves(pos);
	if (moves.empty()) { return check ? -mate_score(ctx.ply) : 0; }

	auto pat = pos.turn == Color::White ? tapered_eval(pos) : -tapered_eval(pos);

	if (!check) {
		if (pat >= beta) { return beta; }
		alpha = std::max(pat, alpha);

		std::erase_if(moves, [&](Move const& m) {
			return !get_bit(pos.occ, m.to) && m.to != pos.en_passant;
		});
	}

	std::ranges::sort(moves, std::greater{}, [&](Move const& m) {
		return move_score(pos, m);
	});

	for (auto const& move : moves) {
		make_move(pos, move);
		++ctx.ply;
		auto eval = -quiescence(pos, -beta, -alpha, ctx);
		--ctx.ply;
		unmake_move(pos, move);

		if (ctx.timed_out) { break; }

		if (eval >= beta) { return beta; }
		alpha = std::max(eval, alpha);
	}

	return alpha;
}
} // namespace CastleMate::engine