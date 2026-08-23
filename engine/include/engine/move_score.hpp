#pragma once
#include "castlemate/core/move.hpp"
#include "engine/evaluation/evaluation.hpp"

namespace CastleMate::engine {
namespace {
constexpr auto piece_on(Position const& pos, int sq) -> Piece {
	for (auto p = 0; p < COUNT_; p++) {
		if (get_bit(pos.bb[p], sq)) { return static_cast<Piece>(p); } // NOLINT
	}
	return COUNT_;
};
} // namespace

// MVV-LVA
inline auto move_score(Position const& pos, Move const& m) {
	if (!get_bit(pos.occ, m.to)) { return 0; }

	auto victim = piece_on(pos, m.to);
	auto attacker = piece_on(pos, m.from);

	return (piece_value_v.at(victim % 6) * 10) - piece_value_v.at(attacker % 6); //
}
} // namespace CastleMate::engine