#pragma once
#include "castlemate/ui/board_view.hpp"

namespace CastleMate {
class App;

class Board {
  public:
	Board(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

  private:
	std::unique_ptr<BoardView> m_boardView{};

	std::uint64_t m_bitboard{};
};
} // namespace CastleMate