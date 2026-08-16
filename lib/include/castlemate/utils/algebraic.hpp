#pragma once
#include "castlemate/core/move.hpp"
#include "castlemate/core/movegen.hpp"
#include <algorithm>

namespace CastleMate {
constexpr std::array<char, COUNT_> piece_char = {
	'P', 'N', 'B', 'R', 'Q', 'K', // white
	'P', 'N', 'B', 'R', 'Q', 'K', // black
};

constexpr std::array<Piece, COUNT_> white_counterpart = {
	WP, WN, WB, WR, WQ, WK, WP, WN, WB, WR, WQ, WK,
};

constexpr std::array<Piece, COUNT_> black_counterpart = {
	BP, BN, BB, BR, BQ, BK, BP, BN, BB, BR, BQ, BK,
};

constexpr std::array<char, COUNT_> promo_char = {
	' ', 'N', 'B', 'R', 'Q', ' ', // white
	' ', 'N', 'B', 'R', 'Q', ' ', // black
};

inline auto needs_disambig(Position& pos, Move m, Piece white_piece, Piece black_piece) -> std::pair<bool, bool> {
	auto is_white = get_bit(pos.white_occ, m.from);
	auto target_piece = is_white ? white_piece : black_piece;

	std::uint64_t others = pos.bb[target_piece] & ~(1ULL << m.from); // NOLINT
	bool file_ambig = false;
	bool rank_ambig = false;

	while (others) {
		auto sq = static_cast<std::uint8_t>(pop_lsb(others));
		auto legal = legal_moves(pos);
		if (std::ranges::contains(legal, m)) {
			if (sq % 8 == m.from % 8) {
				rank_ambig = true;
			} else {
				file_ambig = true;
			}
		}
	}
	return {file_ambig, rank_ambig};
};

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
inline auto to_algebraic(Move m, Position& pos) -> std::string {
	auto piece_on = [&](int sq) -> Piece {
		for (auto p = 0; p < COUNT_; p++) {
			if (get_bit(pos.bb[p], sq)) { return static_cast<Piece>(p); }
		}
		return COUNT_;
	};

	auto piece = piece_on(m.from);

	// castling
	if (piece == WK || piece == BK) {
		std::string castle_str;
		if (m.from == 4 && m.to == 6) {
			castle_str = "O-O";
		} else if (m.from == 4 && m.to == 2) {
			castle_str = "O-O-O";
		} else if (m.from == 60 && m.to == 62) {
			castle_str = "O-O";
		} else if (m.from == 60 && m.to == 58) {
			castle_str = "O-O-O";
		}

		if (!castle_str.empty()) {
			auto undo = make_move(pos, m);
			if (in_checkmate(pos)) {
				castle_str += '#';
			} else if (in_check(pos, pos.turn == Color::White)) {
				castle_str += '+';
			}
			unmake_move(pos, undo);
			return castle_str;
		}
	}

	auto is_capture = get_bit(pos.occ, m.to) || ((piece == WP || piece == BP) && m.to == pos.en_passant);

	auto file_char = [](int sq) {
		return static_cast<char>('a' + (sq % 8));
	};
	auto rank_char = [](int sq) {
		return static_cast<char>('1' + (sq / 8));
	};

	auto ret = std::string{};

	char pc = piece_char.at(piece);
	if (pc != 'P') {
		ret += pc;
		auto [f, r] = needs_disambig(pos, m, white_counterpart.at(piece), black_counterpart.at(piece));
		if (f) { ret += file_char(m.from); }
		if (r) { ret += rank_char(m.from); }
	} else if (is_capture) {
		ret += file_char(m.from);
	}

	if (is_capture) { ret += 'x'; }

	ret += file_char(m.to);
	ret += rank_char(m.to);

	if (m.promotion != COUNT_) {
		ret += '=';
		ret += promo_char.at(m.promotion);
	}

	auto undo = make_move(pos, m);

	if (in_checkmate(pos)) {
		ret += '#';
	} else if (in_check(pos, pos.turn == Color::White)) {
		ret += '+';
	}

	unmake_move(pos, undo);

	return ret;
}
} // namespace CastleMate