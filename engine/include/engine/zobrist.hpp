#pragma once
#include "castlemate/core/position.hpp"
#include "castlemate/utils/bit_math.hpp"
#include <array>
#include <random>

namespace CastleMate::engine {
struct ZobristKeys {
	std::array<std::array<uint64_t, 64>, 12> piece{};
	std::array<uint64_t, 16> castling{};
	std::array<uint64_t, 8> en_passant{};
	uint64_t side{};
};

inline auto init_zobrist() -> ZobristKeys {
	auto rng = std::mt19937_64{123456789};
	auto dist = std::uniform_int_distribution<std::uint64_t>{};
	auto keys = ZobristKeys{};
	for (auto& piece : keys.piece) {
		for (auto& sq : piece) { sq = dist(rng); }
	}
	for (auto& castle : keys.castling) { castle = dist(rng); }
	for (auto& en_passant : keys.en_passant) { en_passant = dist(rng); }
	keys.side = dist(rng);
	return keys;
}

inline auto zobrist() -> ZobristKeys const& {
	static auto const keys = init_zobrist();
	return keys;
}

inline auto compute_hash(Position const& pos) -> std::uint64_t {
	static auto const keys = zobrist();

	auto hash = std::uint64_t{};
	for (std::size_t i = 0; i < COUNT_; ++i) {
		auto bb = pos.bb[i]; // NOLINT
		while (bb) {
			auto sq = pop_lsb(bb);
			hash ^= keys.piece[i][sq]; // NOLINT
		}
	}
	hash ^= keys.castling.at(static_cast<std::size_t>((pos.castle_wk << 0) | (pos.castle_wq << 1) |
													  (pos.castle_bk << 2) | (pos.castle_bq << 3)));
	if (pos.en_passant != -1) { hash ^= keys.en_passant.at(static_cast<std::size_t>(pos.en_passant % 8)); }
	if (pos.turn == Color::Black) { hash ^= keys.side; }
	return hash;
}

} // namespace CastleMate::engine