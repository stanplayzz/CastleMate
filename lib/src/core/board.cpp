#include "castlemate/core/board.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "castlemate/utils/constants.hpp"
#include <print>

namespace CastleMate {
Board::Board() { load_board(); }

void Board::click_square(int sq, SquareOutline& outline, bool white_bottom) {
	if (m_pending_move) { return; }

	auto display_sq = white_bottom ? sq : 63 - sq;

	if (m_selected_sq.has_value()) {
		move({.from = *m_selected_sq, .to = sq});
		m_selected_sq = std::nullopt;
	} else if (get_bit(m_position.occ, sq)) {
		if ((get_bit(m_position.white_occ, sq) && !m_white_turn) ||
			(get_bit(m_position.black_occ, sq) && m_white_turn)) {
			return;
		}
		m_selected_sq = sq;
		auto pos = glm::ivec2{display_sq % 8, display_sq / 8};
		outline.set_position((glm::vec2{pos - glm::ivec2{4, 4}} * tile_size_v) + (tile_size_v * 0.5f));
	}
	outline.should_draw = m_selected_sq.has_value();
}

void Board::set_promotion(Piece p) {
	m_pending_move->promotion = p;
	m_should_promote = false;
	finish_move(*m_pending_move);
	m_pending_move = std::nullopt;
}

void Board::load_board() {
	// White
	for (auto i = 0; i < 8; i++) { set_bit(m_position.bb[WP], sq(1, i)); }
	set_bit(m_position.bb[WR], sq(0, 0));
	set_bit(m_position.bb[WR], sq(0, 7));
	set_bit(m_position.bb[WN], sq(0, 1));
	set_bit(m_position.bb[WN], sq(0, 6));
	set_bit(m_position.bb[WB], sq(0, 2));
	set_bit(m_position.bb[WB], sq(0, 5));
	set_bit(m_position.bb[WQ], sq(0, 3));
	set_bit(m_position.bb[WK], sq(0, 4));

	// Black
	for (auto i = 0; i < 8; i++) { set_bit(m_position.bb[BP], sq(6, i)); }
	set_bit(m_position.bb[BR], sq(7, 0));
	set_bit(m_position.bb[BR], sq(7, 7));
	set_bit(m_position.bb[BN], sq(7, 1));
	set_bit(m_position.bb[BN], sq(7, 6));
	set_bit(m_position.bb[BB], sq(7, 2));
	set_bit(m_position.bb[BB], sq(7, 5));
	set_bit(m_position.bb[BQ], sq(7, 3));
	set_bit(m_position.bb[BK], sq(7, 4));

	update_occ();
}

void Board::update_occ() {
	auto const& bb = m_position.bb;
	m_position.white_occ = bb[WP] | bb[WR] | bb[WN] | bb[WB] | bb[WQ] | bb[WK];
	m_position.black_occ = bb[BP] | bb[BR] | bb[BN] | bb[BB] | bb[BQ] | bb[BK];
	m_position.occ = m_position.white_occ | m_position.black_occ;
}

void Board::move(Move m) {
	auto moves = get_legal_moves(m_position, m.from);
	if (std::ranges::find(moves, m.to) == moves.end()) { return; }

	// promotion
	auto is_white = get_bit(m_position.bb[WP], m.from);
	auto is_black = get_bit(m_position.bb[BP], m.from);
	if ((is_white && m.to >= 56) || (is_black && m.to < 8)) {
		m_pending_move = m;
		m_should_promote = true;
		return;
	}

	finish_move(m);
}

void Board::finish_move(Move m) {
	apply_move(m_position, m);
	m_white_turn = !m_white_turn;
	m_update_view = true;

	if (in_checkmate(m_position, m_white_turn)) {
		m_ending.white_won = !m_white_turn;
		m_has_ended = true;
	}
	if (in_stalemate(m_position, m_white_turn)) {
		m_ending.draw = true;
		m_has_ended = true;
	}
}
} // namespace CastleMate