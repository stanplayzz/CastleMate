#pragma once
#include "castlemate/core/position.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "engine/piece_square_table.hpp"
#include <array>

namespace CastleMate::engine {
inline constexpr auto piece_value_v = std::array{
	100, 320, 330, 500, 900, 20000,
};

inline constexpr auto phase_weight_v = std::array{0, 1, 1, 2, 4, 0};
inline constexpr auto max_phase_v = 24;

inline auto tapered_eval(Position const& pos) {
	auto mg = 0;
	auto eg = 0;
	auto phase = 0;

	for (std::size_t p = 0; p < COUNT_; ++p) {
		auto const type = p % 6;
		auto const white = p < 6;
		auto const value = piece_value_v.at(type);

		auto bb = pos.bb[p]; // NOLINT
		while (bb) {
			auto sq = pop_lsb(bb);
			auto table_sq = white ? sq : (sq ^ 56);

			auto mg_score = value + mg_pesto_table.at(type).at(table_sq);
			auto eg_score = value + eg_pesto_table.at(type).at(table_sq);

			mg += white ? mg_score : -mg_score;
			eg += white ? eg_score : -eg_score;

			phase += phase_weight_v.at(type);
		}
	}

	phase = std::min(phase, max_phase_v);

	return ((mg * phase) + (eg * (max_phase_v - phase))) / max_phase_v;
}
} // namespace CastleMate::engine