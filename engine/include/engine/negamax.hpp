#pragma once
#include "engine/quiescence.hpp"

namespace CastleMate::engine {
namespace {
constexpr auto same_move(Move const& a, Move const& b) { return a.from == b.from && a.to == b.to; }

inline void store_killer(SearchContext& ctx, Move const& m) {
	if (ctx.ply < 0 || static_cast<std::size_t>(ctx.ply) >= ctx.killers.size()) { return; }
	auto& slot = ctx.killers.at(static_cast<std::size_t>(ctx.ply));
	if (!same_move(slot[0], m)) {
		slot[1] = slot[0];
		slot[0] = m;
	}
}
} // namespace

struct NegamaxResult {
	int eval{};
	Move move{};
};

inline auto negamax(Position& pos, int depth, int alpha, int beta, SearchContext& ctx) -> NegamaxResult {
	if (ctx.check_time()) { return {}; }
	if (ctx.ply > 0 && is_draw(pos)) { return {}; }

	auto original_alpha = alpha;

	auto* entry = ctx.tt.probe(pos.hash);
	auto const tt_move = entry ? entry->move : Move{};
	if (entry && entry->depth >= depth) {
		int score = score_from_tt(entry->score, ctx.ply);
		if (entry->bound == Bound::EXACT) { return {.eval = score, .move = entry->move}; }
		if (entry->bound == Bound::LOWER) { alpha = std::max(alpha, score); }
		if (entry->bound == Bound::UPPER) { beta = std::min(beta, score); }
		if (alpha >= beta) { return {.eval = score, .move = entry->move}; }
	}

	auto const check = in_check(pos, pos.turn == Color::White);
	if (depth <= 0) {
		if (!check) { return {.eval = quiescence(pos, alpha, beta, ctx)}; }
		depth = 1;
	}

	auto legal = legal_moves(pos);
	if (legal.empty()) {
		if (check) { return {.eval = -mate_score(ctx.ply)}; }
		return {};
	}

	auto const killer = (static_cast<std::size_t>(ctx.ply) < ctx.killers.size())
							? ctx.killers.at(static_cast<std::size_t>(ctx.ply))
							: std::array<Move, 2>{};

	std::ranges::sort(legal, std::greater{}, [&](Move const& m) {
		if (same_move(m, tt_move)) { return 1'000'000'000; }
		auto const score = move_score(pos, m);
		if (score == 0 && (same_move(m, killer[0]) || same_move(m, killer[1]))) { return 1; }
		return score;
	});

	auto best = NegamaxResult{.eval = -infinity_v, .move = legal.front()};
	for (auto move : legal) {
		make_move(pos, move);
		++ctx.ply;
		auto eval = -negamax(pos, depth - 1, -beta, -alpha, ctx).eval;
		--ctx.ply;
		unmake_move(pos, move);

		if (ctx.timed_out) { break; }

		if (eval > best.eval) { best = {.eval = eval, .move = move}; }

		alpha = std::max(alpha, eval);
		if (alpha >= beta) {
			if (!get_bit(pos.occ, move.to)) { store_killer(ctx, move); }
			break; // Bad branch
		}
	}

	if (!ctx.timed_out) {
		auto bound = best.eval <= original_alpha ? Bound::UPPER
					 : best.eval >= beta		 ? Bound::LOWER // NOLINT
												 : Bound::EXACT;
		ctx.tt.store(pos.hash, depth, score_to_tt(best.eval, ctx.ply), bound, best.move);
	}

	return best;
}
} // namespace CastleMate::engine