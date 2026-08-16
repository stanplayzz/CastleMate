#pragma once
#include "castlemate/core/move.hpp"

namespace CastleMate {
struct Undo {
	Position pos;
	std::optional<Piece> captured{};
};

inline auto make_move(Position& pos, Move m) -> Undo {
	auto old_pos = pos;

	auto is_white = get_bit(pos.bb[WP], m.from);
	auto is_black = get_bit(pos.bb[BP], m.from);

	std::optional<Piece> captured;

	for (std::size_t i = 0; i < COUNT_; ++i) {
		if (get_bit(pos.bb[i], m.to)) { // NOLINT
			captured = static_cast<Piece>(i);
			break;
		}
	}

	for (auto& bb : pos.bb) { clear_bit(bb, m.to); }

	if (is_white && m.to == pos.en_passant) {
		clear_bit(pos.bb[BP], m.to - 8);
		clear_bit(pos.occ, m.to - 8);
		captured = BP;
	}
	if (is_black && m.to == pos.en_passant) {
		clear_bit(pos.bb[WP], m.to + 8);
		clear_bit(pos.occ, m.to + 8);
		captured = WP;
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

	pos.turn = pos.turn == Color::White ? Color::Black : Color::White;

	return {.pos = old_pos, .captured = captured};
}

inline void unmake_move(Position& pos, Undo const& undo) { pos = undo.pos; }

inline auto legal_moves(Position& pos) -> std::vector<Move> {
	auto moves = std::vector<Move>{};
	auto const white = pos.turn == Color::White;
	auto const friendly = white ? pos.white_occ : pos.black_occ;

	auto pieces = friendly;
	while (pieces) {
		auto const from = static_cast<std::uint8_t>(pop_lsb(pieces));
		auto pseudo = get_moves(pos, from, friendly);
		pseudo &= ~pos.bb[white ? BK : WK]; // NOLINT

		// NOLINTNEXTLINE
		if (get_bit(pos.bb[white ? WK : BK], from)) { pseudo |= castle_moves(pos); }

		while (pseudo) {
			auto const to = static_cast<std::uint8_t>(pop_lsb(pseudo));
			auto move = Move{.from = from, .to = to};

			auto const is_pawn = white ? get_bit(pos.bb[WP], from) : get_bit(pos.bb[BP], from);
			auto undo = make_move(pos, move);

			if (!in_check(pos, white)) {
				if (white && to >= 56 && is_pawn) {
					moves.push_back({.from = from, .to = to, .promotion = WQ});
					moves.push_back({.from = from, .to = to, .promotion = WR});
					moves.push_back({.from = from, .to = to, .promotion = WB});
					moves.push_back({.from = from, .to = to, .promotion = WN});
				} else if (!white && to < 8 && is_pawn) {
					moves.push_back({.from = from, .to = to, .promotion = BQ});
					moves.push_back({.from = from, .to = to, .promotion = BR});
					moves.push_back({.from = from, .to = to, .promotion = BB});
					moves.push_back({.from = from, .to = to, .promotion = BN});
				} else {
					moves.push_back(move);
				}
			}
			unmake_move(pos, undo);
		}
	}

	return moves;
}

inline auto in_checkmate(Position& pos) { return in_check(pos, pos.turn == Color::White) && legal_moves(pos).empty(); }

inline auto in_stalemate(Position& pos) { return !in_check(pos, pos.turn == Color::White) && legal_moves(pos).empty(); }
} // namespace CastleMate