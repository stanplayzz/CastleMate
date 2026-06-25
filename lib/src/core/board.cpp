#include "castlemate/core/board.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "castlemate/utils/constants.hpp"
#include <print>

namespace CastleMate {
Board::Board() {
	std::println("Started precomputing moves");
	queen_move(0, 0);
	std::println("Finished precomputing moves");
	load_board();
}

void Board::click_square(glm::ivec2 square, SquareOutline& outline) {
	auto i = (square.y * 8) + square.x;
	if (get_bit(m_position.occ, i) && !(m_selected_sq.has_value() && m_selected_sq == i)) {
		m_selected_sq = i;
		outline.set_position((glm::vec2{square - glm::ivec2{4, 4}} * tile_size_v) + tile_size_v * 0.5f);
	} else if (m_selected_sq.has_value()) {
		move({.from = *m_selected_sq, .to = i});
		m_selected_sq = std::nullopt;
	}
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

	std::uint64_t* bb = nullptr;
	for (auto& b : m_position.bb) {
		if (get_bit(b, m.from)) {
			bb = &b;
			break;
		}
	}
	if (!bb) { return; }

	clear_bit(*bb, m.from);
	set_bit(*bb, m.to);

	update_occ();

	m_update_view = true;
}
} // namespace CastleMate