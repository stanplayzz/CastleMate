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
			make_move(pos, m);
			if (in_checkmate(pos)) {
				castle_str += '#';
			} else if (in_check(pos, pos.turn == Color::White)) {
				castle_str += '+';
			}
			unmake_move(pos, m);
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

	make_move(pos, m);

	if (in_checkmate(pos)) {
		ret += '#';
	} else if (in_check(pos, pos.turn == Color::White)) {
		ret += '+';
	}

	unmake_move(pos, m);

	return ret;
}

// UCI long algebraic notation
inline auto to_uci(Move m) -> std::string {
	auto file_char = [](int sq) {
		return static_cast<char>('a' + (sq % 8));
	};
	auto rank_char = [](int sq) {
		return static_cast<char>('1' + (sq / 8));
	};

	std::string ret;
	ret += file_char(m.from);
	ret += rank_char(m.from);
	ret += file_char(m.to);
	ret += rank_char(m.to);

	if (m.promotion != COUNT_) { ret += static_cast<char>(std::tolower(promo_char.at(m.promotion))); }
	return ret;
}

inline auto from_uci(std::string const& uci, Position& pos) -> std::optional<Move> {
	if (uci.size() < 4) { return std::nullopt; }

	auto sq = [](char file, char rank) -> std::uint8_t {
		return static_cast<std::uint8_t>(((rank - '1') * 8) + (file - 'a'));
	};

	auto from = sq(uci[0], uci[1]);
	auto to = sq(uci[2], uci[3]);

	Piece promo = COUNT_;
	if (uci.size() == 5) {
		char p = static_cast<char>(std::toupper(uci[4]));
		bool const white = pos.turn == Color::White;
		switch (p) {
		case 'Q': promo = white ? WQ : BQ; break;
		case 'R': promo = white ? WR : BR; break;
		case 'B': promo = white ? WB : BB; break;
		case 'N': promo = white ? WN : BN; break;
		default: return std::nullopt;
		}
	}

	for (auto const& m : legal_moves(pos)) {
		if (m.from == from && m.to == to && m.promotion == promo) { return m; }
	}

	return std::nullopt; // illegal or malformed
}
} // namespace CastleMate