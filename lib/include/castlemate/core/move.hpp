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
	Piece promotion{COUNT_};
};

constexpr std::uint64_t FILE_A = 0x0101010101010101;
constexpr std::uint64_t FILE_B = 0x0202020202020202;
constexpr std::uint64_t FILE_G = 0x4040404040404040;
constexpr std::uint64_t FILE_H = 0x8080808080808080;
constexpr std::uint64_t RANK_1 = 0x00000000000000ff;
constexpr std::uint64_t RANK_2 = 0x000000000000ff00;
constexpr std::uint64_t RANK_7 = 0x00ff000000000000;
constexpr std::uint64_t RANK_8 = 0xff00000000000000;

// Precomputed piece moves
constexpr auto knight_move(int sq, std::uint64_t friendly) -> std::uint64_t {
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
	return moves_v.at(static_cast<std::size_t>(sq)) & ~friendly;
}

constexpr auto king_move(int sq, std::uint64_t friendly) -> std::uint64_t {
	static constexpr auto moves_v = []() {
		auto moves = std::array<std::uint64_t, 64>{};
		for (std::size_t i = 0; i < 64; i++) {
			std::uint64_t b = 1ULL << i;
			moves.at(i) = (b << 8) | (b >> 8) | ((b << 1) & ~FILE_A) | ((b >> 1) & ~FILE_H) | ((b << 7) & ~FILE_H) |
						  ((b >> 7) & ~FILE_A) | ((b << 9) & ~FILE_A) | ((b >> 9) & ~FILE_H);
		}
		return moves;
	}();
	return moves_v.at(static_cast<std::size_t>(sq)) & ~friendly;
}

constexpr auto pawn_move(int sq, Piece p, Position pos, std::uint64_t friendly) {
	std::uint64_t b = 1ULL << sq;
	std::uint64_t ep_bb = (pos.en_passant >= 0) ? (1ULL << pos.en_passant) : 0;

	if (p == WP) {
		auto single_push = (b << 8) & ~pos.occ;
		auto double_push = get_bit(RANK_2, sq) ? (single_push << 8) & ~pos.occ : 0;
		auto attacks = ((b << 9) & ~FILE_A) | ((b << 7) & ~FILE_H);
		return single_push | double_push | (attacks & ((~friendly & pos.occ) | ep_bb));
	}
	auto single_push = (b >> 8) & ~pos.occ;
	auto double_push = get_bit(RANK_7, sq) ? (single_push >> 8) & ~pos.occ : 0;
	auto attacks = ((b >> 9) & ~FILE_H) | ((b >> 7) & ~FILE_A);
	return single_push | double_push | (attacks & ((~friendly & pos.occ) | ep_bb));
}

constexpr auto rook_move(int sq, std::uint64_t occ, std::uint64_t friendly) {
	static auto const magics = init_magics(false);
	auto const& m = magics.at(static_cast<std::size_t>(sq));
	return m.table[((occ & m.mask) * m.magic) >> m.shift] & ~friendly;
}

constexpr auto bishop_move(int sq, std::uint64_t occ, std::uint64_t friendly) {
	static auto const magics = init_magics(true);
	auto const& m = magics.at(static_cast<std::size_t>(sq));
	return m.table[((occ & m.mask) * m.magic) >> m.shift] & ~friendly;
}

constexpr auto queen_move(int sq, std::uint64_t occ, std::uint64_t friendly) {
	return rook_move(sq, occ, friendly) | bishop_move(sq, occ, friendly);
}

constexpr auto square_attacked(Position const& pos, int sq, bool by_black) -> bool {
	auto e_pawns = by_black ? pos.bb[BP] : pos.bb[WP];
	auto e_knights = by_black ? pos.bb[BN] : pos.bb[WN];
	auto e_bishops = by_black ? pos.bb[BB] : pos.bb[WB];
	auto e_rooks = by_black ? pos.bb[BR] : pos.bb[WR];
	auto e_queens = by_black ? pos.bb[BQ] : pos.bb[WQ];
	auto e_king = by_black ? pos.bb[BK] : pos.bb[WK];

	if (knight_move(sq, 0) & e_knights) { return true; }
	if (king_move(sq, 0) & e_king) { return true; }
	if (rook_move(sq, pos.occ, 0) & (e_rooks | e_queens)) { return true; }
	if (bishop_move(sq, pos.occ, 0) & (e_bishops | e_queens)) { return true; }

	std::uint64_t b = 1ULL << sq;
	auto pawn_attacks =
		by_black ? ((b << 9) & ~FILE_H) | ((b << 7) & ~FILE_A) : ((b >> 9) & ~FILE_A) | ((b >> 7) & ~FILE_H);
	return (pawn_attacks & e_pawns) != 0;
}

constexpr auto in_check(Position const& pos, bool white) {
	auto sq = std::countr_zero(white ? pos.bb[WK] : pos.bb[BK]);
	return square_attacked(pos, sq, white);
}

constexpr auto castle_moves(Position const& pos, bool white) -> std::uint64_t {
	std::uint64_t ret = 0;

	if (white) {
		if (pos.castle_wk && !get_bit(pos.occ, 5) && !get_bit(pos.occ, 6) && !in_check(pos, true) &&
			!square_attacked(pos, 5, true) && !square_attacked(pos, 6, true)) {
			set_bit(ret, 6);
		}
		if (pos.castle_wq && !get_bit(pos.occ, 1) && !get_bit(pos.occ, 2) && !get_bit(pos.occ, 3) &&
			!in_check(pos, true) && !square_attacked(pos, 1, true) && !square_attacked(pos, 2, true)) {
			set_bit(ret, 2);
		}
	} else {
		if (pos.castle_bk && !get_bit(pos.occ, 61) && !get_bit(pos.occ, 62) && !in_check(pos, false) &&
			!square_attacked(pos, 61, false) && !square_attacked(pos, 62, false)) {
			set_bit(ret, 62);
		}
		if (pos.castle_bq && !get_bit(pos.occ, 57) && !get_bit(pos.occ, 58) && !get_bit(pos.occ, 59) &&
			!in_check(pos, false) && !square_attacked(pos, 57, false) && !square_attacked(pos, 58, false)) {
			set_bit(ret, 58);
		}
	}

	return ret;
}

constexpr auto get_moves(Position pos, int sq, std::uint64_t friendly) -> std::uint64_t {
	auto moves = std::vector<int>{};
	if (get_bit(pos.bb[WN], sq) || get_bit(pos.bb[BN], sq)) { return knight_move(sq, friendly); }
	if (get_bit(pos.bb[WK], sq) || get_bit(pos.bb[BK], sq)) { return king_move(sq, friendly); }
	if (get_bit(pos.bb[WP], sq)) { return pawn_move(sq, WP, pos, friendly); }
	if (get_bit(pos.bb[BP], sq)) { return pawn_move(sq, BP, pos, friendly); }
	if (get_bit(pos.bb[WR], sq) || get_bit(pos.bb[BR], sq)) { return rook_move(sq, pos.occ, friendly); }
	if (get_bit(pos.bb[WB], sq) || get_bit(pos.bb[BB], sq)) { return bishop_move(sq, pos.occ, friendly); }
	if (get_bit(pos.bb[WQ], sq) || get_bit(pos.bb[BQ], sq)) { return queen_move(sq, pos.occ, friendly); }
	return 0;
}

constexpr auto update_castling_rights(Position& pos, Move m) {
	if (m.from == 0 || m.to == 0) { pos.castle_wq = false; }
	if (m.from == 7 || m.to == 7) { pos.castle_wk = false; }
	if (m.from == 56 || m.to == 56) { pos.castle_bq = false; }
	if (m.from == 63 || m.to == 63) { pos.castle_bk = false; }
}

constexpr auto castle(Position& pos, Move m) {
	if (get_bit(pos.bb[WK], m.to)) {
		if (m.from == 4 && m.to == 6) {
			replace_bit(pos.bb[WR], 7, 5);
		} else if (m.from == 4 && m.to == 2) {
			replace_bit(pos.bb[WR], 0, 3);
		}
		pos.castle_wk = pos.castle_wq = false;
	}
	if (get_bit(pos.bb[BK], m.to)) {
		if (m.from == 60 && m.to == 62) {
			replace_bit(pos.bb[BR], 63, 61);
		} else if (m.from == 60 && m.to == 58) {
			replace_bit(pos.bb[BR], 56, 59);
		}
		pos.castle_bk = pos.castle_bq = false;
	}

	update_castling_rights(pos, m);
}

constexpr auto promote(Position& pos, Move m, bool white) {
	clear_bit(white ? pos.bb[WP] : pos.bb[BP], m.from);
	set_bit(pos.bb[m.promotion], m.to); // NOLINT
}

constexpr auto apply_move(Position& pos, Move m) {
	auto is_white = get_bit(pos.bb[WP], m.from);
	auto is_black = get_bit(pos.bb[BP], m.from);

	auto captured = get_bit(pos.occ, m.to);

	for (auto& bb : pos.bb) { clear_bit(bb, m.to); }

	if (is_white && m.to == pos.en_passant) {
		clear_bit(pos.bb[BP], m.to - 8);
		clear_bit(pos.occ, m.to - 8);
		captured = true;
	}
	if (is_black && m.to == pos.en_passant) {
		clear_bit(pos.bb[WP], m.to + 8);
		clear_bit(pos.occ, m.to + 8);
		captured = true;
	}

	for (auto& bb : pos.bb) {
		if (get_bit(bb, m.from)) {
			if (((is_white && m.to >= 56) || (is_black && m.to < 8)) && m.promotion != COUNT_) {
				promote(pos, m, is_white);
				break;
			}
			replace_bit(bb, m.from, m.to);
			break;
		}
	}

	// only castles if needed
	castle(pos, m);

	pos.en_passant = -1;
	if (is_white && (m.to - m.from == 16)) { pos.en_passant = m.from + 8; }
	if (is_black && (m.from - m.to == 16)) { pos.en_passant = m.from - 8; }

	auto const& bb = pos.bb;
	pos.white_occ = bb[WP] | bb[WR] | bb[WN] | bb[WB] | bb[WQ] | bb[WK];
	pos.black_occ = bb[BP] | bb[BR] | bb[BN] | bb[BB] | bb[BQ] | bb[BK];
	pos.occ = pos.white_occ | pos.black_occ;

	return captured;
}

constexpr auto get_legal_moves(Position pos, int sq) -> std::vector<int> {
	auto moves = std::vector<int>{};
	auto friendly = get_bit(pos.white_occ, sq) ? pos.white_occ : pos.black_occ;
	auto white = get_bit(pos.white_occ, sq);

	auto pseudo = get_moves(pos, sq, friendly);
	if (get_bit(white ? pos.bb[WK] : pos.bb[BK], sq)) { pseudo |= castle_moves(pos, white); }
	while (pseudo) {
		auto to = static_cast<int>(pop_lsb(pseudo));
		auto temp = pos;
		apply_move(temp, {.from = sq, .to = to});
		if (!in_check(temp, white)) { moves.push_back(to); }
	}

	return moves;
}

constexpr auto in_checkmate(Position const& pos, bool white) {
	if (!in_check(pos, white)) { return false; }

	auto friendly = white ? pos.white_occ : pos.black_occ;
	auto temp = friendly;
	while (temp) {
		auto sq = static_cast<int>(pop_lsb(temp));
		if (!get_legal_moves(pos, sq).empty()) { return false; }
	}
	return true;
}

constexpr auto in_stalemate(Position const& pos, bool white) {
	if (in_check(pos, white)) { return false; }

	auto friendly = white ? pos.white_occ : pos.black_occ;
	auto temp = friendly;
	while (temp) {
		auto sq = static_cast<int>(pop_lsb(temp));
		if (!get_legal_moves(pos, sq).empty()) { return false; }
	}
	return true;
}
} // namespace CastleMate