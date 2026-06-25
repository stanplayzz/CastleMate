#pragma once
#include "castlemate/core/magic.hpp"
#include "castlemate/core/position.hpp"
#include "castlemate/utils/bit_math.hpp"
#include <array>
#include <vector>

namespace CastleMate {
struct Move {
	int from{};
	int to{};
};

constexpr std::uint64_t FILE_A = 0x0101010101010101;
constexpr std::uint64_t FILE_B = 0x0202020202020202;
constexpr std::uint64_t FILE_G = 0x4040404040404040;
constexpr std::uint64_t FILE_H = 0x8080808080808080;
constexpr std::uint64_t RANK_2 = 0x000000000000ff00;
constexpr std::uint64_t RANK_7 = 0x00ff000000000000;

// Precomputed piece moves
constexpr auto knight_move(int sq) -> std::uint64_t {
	static constexpr auto moves_v = []() {
		auto moves = std::array<std::uint64_t, 64>{};
		for (std::size_t i = 0; i < 64; i++) {
			std::uint64_t b = 1ULL << i;
			moves.at(i) = ((b << 17) & ~FILE_A) | ((b << 15) & ~FILE_H) | ((b << 10) & ~FILE_A & ~FILE_B) |
						  ((b << 6) & ~FILE_G & ~FILE_H) | ((b >> 17) & ~FILE_H) | ((b >> 15) & ~FILE_A) |
						  ((b >> 10) & ~FILE_G & ~FILE_H) | ((b >> 6) & ~FILE_A & ~FILE_B);
		}
		return moves;
	}();
	return moves_v.at(static_cast<std::size_t>(sq));
}

constexpr auto king_move(int sq) -> std::uint64_t {
	static constexpr auto moves_v = []() {
		auto moves = std::array<std::uint64_t, 64>{};
		for (std::size_t i = 0; i < 64; i++) {
			std::uint64_t b = 1ULL << i;
			moves.at(i) = (b << 8) | (b >> 8) | ((b << 1) & ~FILE_A) | ((b >> 1) & ~FILE_H) | ((b << 7) & ~FILE_H) |
						  ((b >> 7) & ~FILE_A) | ((b << 9) & ~FILE_A) | ((b >> 9) & ~FILE_H);
		}
		return moves;
	}();
	return moves_v.at(static_cast<std::size_t>(sq));
}

constexpr auto pawn_move(int sq, Piece p, std::uint64_t occ) {
	std::uint64_t b = 1ULL << sq;
	if (p == WP) {
		auto single_push = (b << 8) & ~occ;
		auto double_push = get_bit(RANK_2, sq) ? (single_push << 8) & ~occ : 0;
		return single_push | double_push;
	}
	auto single_push = (b >> 8) & ~occ;
	auto double_push = get_bit(RANK_7, sq) ? (single_push >> 8) & ~occ : 0;
	return single_push | double_push;
}

constexpr auto rook_move(int sq, std::uint64_t occ) {
	static auto const magics = init_magics(false);
	auto const& m = magics.at(static_cast<std::size_t>(sq));
	return m.table[((occ & m.mask) * m.magic) >> m.shift];
}

constexpr auto bishop_move(int sq, std::uint64_t occ) {
	static auto const magics = init_magics(true);
	auto const& m = magics.at(static_cast<std::size_t>(sq));
	return m.table[((occ & m.mask) * m.magic) >> m.shift];
}

constexpr auto queen_move(int sq, std::uint64_t occ) { return rook_move(sq, occ) | bishop_move(sq, occ); }

constexpr auto get_moves(Position pos, int sq) -> std::uint64_t {
	auto moves = std::vector<int>{};
	if (get_bit(pos.bb[WN], sq) || get_bit(pos.bb[BN], sq)) { return knight_move(sq); }
	if (get_bit(pos.bb[WK], sq) || get_bit(pos.bb[BK], sq)) { return king_move(sq); }
	if (get_bit(pos.bb[WP], sq)) { return pawn_move(sq, WP, pos.occ); }
	if (get_bit(pos.bb[BP], sq)) { return pawn_move(sq, BP, pos.occ); }
	if (get_bit(pos.bb[WR], sq) || get_bit(pos.bb[BR], sq)) { return rook_move(sq, pos.occ); }
	if (get_bit(pos.bb[WB], sq) || get_bit(pos.bb[BB], sq)) { return bishop_move(sq, pos.occ); }
	if (get_bit(pos.bb[WQ], sq) || get_bit(pos.bb[BQ], sq)) { return queen_move(sq, pos.occ); }
	return 0;
}

constexpr auto get_current_bb(Position& pos, int sq) -> std::uint64_t* {
	std::uint64_t* bb = nullptr;
	for (auto& b : pos.bb) {
		if (get_bit(b, sq)) {
			bb = &b;
			break;
		}
	}
	return bb;
}

constexpr auto get_legal_moves(Position pos, int sq) -> std::vector<int> {
	auto moves = std::vector<int>{};
	auto m = get_moves(pos, sq);
	while (m) { moves.push_back(pop_lsb(m)); } // NOLINT

	return moves;
}
} // namespace CastleMate