#include "castlemate/core/board.hpp"

namespace CastleMate {
Board::Board(gsl::not_null<App const*> app) { m_boardView = std::make_unique<BoardView>(app); }

void Board::draw(le::IRenderer& renderer) const { m_boardView->draw(renderer); }
} // namespace CastleMate