#include "castlemate/core/board.hpp"
#include "castlemate/utils/bit_math.hpp"

namespace CastleMate {
Board::Board(gsl::not_null<App const*> app) {
	m_boardView = std::make_unique<BoardView>(app);

	load_board();
	m_boardView->update_board(static_cast<std::uint64_t const*>(m_position.bb));
}

void Board::draw(le::IRenderer& renderer) const { m_boardView->draw(renderer); }

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
}
} // namespace CastleMate