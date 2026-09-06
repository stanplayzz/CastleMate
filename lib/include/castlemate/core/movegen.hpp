#pragma once
#include "castlemate/core/move.hpp"
#include "engine/zobrist.hpp"

namespace CastleMate {
struct Undo {
	Position pos;
	std::optional<Piece> captured{};
};

inline auto castling_index(Position const& pos) {
	return static_cast<std::size_t>((pos.castle_wk << 0) | (pos.castle_wq << 1) | (pos.castle_bk << 2) |
									(pos.castle_bq << 3));
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
inline auto make_move(Position& pos, Move m) {
	auto& new_state = pos.states.emplace_back();
	new_state.previous = pos.state;
	new_state.prev_en_passant = pos.en_passant;
	new_state.prev_castle_wk = pos.castle_wk;
	new_state.prev_castle_wq = pos.castle_wq;
	new_state.prev_castle_bk = pos.castle_bk;
	new_state.prev_castle_bq = pos.castle_bq;
	new_state.prev_halfmove_clock = pos.halfmove_clock;
	new_state.prev_hash = pos.hash;
	pos.state = &new_state;

	auto const& keys = engine::zobrist();

	if (pos.en_passant != -1) { pos.hash ^= keys.en_passant[static_cast<std::size_t>(pos.en_passant % 8)]; } // NOLINT
	pos.hash ^= keys.castling[castling_index(pos)];															 // NOLINT

	auto moving = Piece{COUNT_};
	for (std::size_t i = 0; i < COUNT_; ++i) {
		if (get_bit(pos.bb[i], m.from)) { // NOLINT
			moving = static_cast<Piece>(i);
			break;
		}
	}
	new_state.moved = moving;

	pos.hash ^= keys.piece[moving][m.from]; // NOLINT

	// Capture
	for (std::size_t i = 0; i < COUNT_; ++i) {
		if (get_bit(pos.bb[i], m.to)) { // NOLINT
			new_state.captured = static_cast<Piece>(i);
			new_state.captured_sq = m.to;
			clear_bit(pos.bb[i], m.to);		 // NOLINT
			pos.hash ^= keys.piece[i][m.to]; // NOLINT
			break;
		}
	}

	// Move
	clear_bit(pos.bb[moving], m.from);	  // NOLINT
	set_bit(pos.bb[moving], m.to);		  // NOLINT
	pos.hash ^= keys.piece[moving][m.to]; // NOLINT

	// En passant
	if ((moving == WP || moving == BP) && m.to == pos.en_passant) {
		std::uint8_t const square = moving == WP ? m.to - 8 : m.to + 8;
		auto const piece = moving == WP ? BP : WP;

		clear_bit(pos.bb[piece], square); // NOLINT

		new_state.captured = piece;
		new_state.captured_sq = square;

		pos.hash ^= keys.piece[piece][square]; // NOLINT
	}

	// Promotion
	if (m.promotion != COUNT_) {
		clear_bit(pos.bb[moving], m.to);		   // NOLINT
		set_bit(pos.bb[m.promotion], m.to);		   // NOLINT
		pos.hash ^= keys.piece[moving][m.to];	   // NOLINT
		pos.hash ^= keys.piece[m.promotion][m.to]; // NOLINT
	}

	castle(pos, m);
	if (m.from == 4 && m.to == 6) {
		pos.hash ^= keys.piece[WR][7] ^ keys.piece[WR][5];
	} else if (m.from == 4 && m.to == 2) {
		pos.hash ^= keys.piece[WR][0] ^ keys.piece[WR][3];
	} else if (m.from == 60 && m.to == 62) {
		pos.hash ^= keys.piece[BR][63] ^ keys.piece[BR][61];
	} else if (m.from == 60 && m.to == 58) {
		pos.hash ^= keys.piece[BR][56] ^ keys.piece[BR][59];
	}

	pos.hash ^= keys.castling[castling_index(pos)]; // NOLINT

	pos.en_passant = -1;
	if (moving == WP && (m.to - m.from == 16)) { pos.en_passant = m.from + 8; }
	if (moving == BP && (m.from - m.to == 16)) { pos.en_passant = m.from - 8; }
	if (pos.en_passant != -1) {
		pos.hash ^= keys.en_passant[static_cast<std::size_t>(pos.en_passant % 8)]; // NOLINT
	}

	auto const& bb = pos.bb;
	pos.white_occ = bb[WP] | bb[WR] | bb[WN] | bb[WB] | bb[WQ] | bb[WK];
	pos.black_occ = bb[BP] | bb[BR] | bb[BN] | bb[BB] | bb[BQ] | bb[BK];
	pos.occ = pos.white_occ | pos.black_occ;

	new_state.irreversible = (new_state.captured != COUNT_) || moving == WP || moving == BP;
	pos.halfmove_clock = new_state.irreversible ? 0 : pos.halfmove_clock + 1;

	pos.turn = pos.turn == Color::White ? Color::Black : Color::White;
	pos.hash ^= keys.side;

	new_state.hash = pos.hash;
}

inline void unmake_move(Position& pos, Move m) {
	auto const& old_state = *pos.state;
	auto const& keys = engine::zobrist();

	if (pos.en_passant != -1) { pos.hash ^= keys.en_passant[static_cast<std::size_t>(pos.en_passant % 8)]; } // NOLINT

	pos.hash ^= keys.castling[castling_index(pos)]; // NOLINT

	pos.hash ^= keys.side;
	pos.turn = pos.turn == Color::White ? Color::Black : Color::White;

	uncastle(pos, m, old_state.moved);

	if (m.promotion != COUNT_) {
		clear_bit(pos.bb[m.promotion], m.to);	   // NOLINT
		pos.hash ^= keys.piece[m.promotion][m.to]; // NOLINT
	} else {
		clear_bit(pos.bb[old_state.moved], m.to);	   // NOLINT
		pos.hash ^= keys.piece[old_state.moved][m.to]; // NOLINT
	}

	set_bit(pos.bb[old_state.moved], m.from);		 // NOLINT
	pos.hash ^= keys.piece[old_state.moved][m.from]; // NOLINT

	if (old_state.captured != COUNT_) {
		set_bit(pos.bb[old_state.captured], old_state.captured_sq);		   // NOLINT
		pos.hash ^= keys.piece[old_state.captured][old_state.captured_sq]; // NOLINT
	}

	pos.en_passant = old_state.prev_en_passant;
	if (pos.en_passant != -1) { pos.hash ^= keys.en_passant[static_cast<std::size_t>(pos.en_passant % 8)]; } // NOLINT

	pos.castle_wk = old_state.prev_castle_wk;
	pos.castle_wq = old_state.prev_castle_wq;
	pos.castle_bk = old_state.prev_castle_bk;
	pos.castle_bq = old_state.prev_castle_bq;
	pos.halfmove_clock = old_state.prev_halfmove_clock;

	pos.hash ^= keys.castling[castling_index(pos)]; // NOLINT

	auto const& bb = pos.bb;
	pos.white_occ = bb[WP] | bb[WR] | bb[WN] | bb[WB] | bb[WQ] | bb[WK];
	pos.black_occ = bb[BP] | bb[BR] | bb[BN] | bb[BB] | bb[BQ] | bb[BK];
	pos.occ = pos.white_occ | pos.black_occ;

	pos.state = old_state.previous;
	pos.states.pop_back();
	pos.hash = old_state.prev_hash;
}

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
			make_move(pos, move);

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
			unmake_move(pos, move);
		}
	}

	return moves;
}

inline auto in_checkmate(Position& pos) { return in_check(pos, pos.turn == Color::White) && legal_moves(pos).empty(); }

inline auto in_stalemate(Position& pos) { return !in_check(pos, pos.turn == Color::White) && legal_moves(pos).empty(); }
} // namespace CastleMate