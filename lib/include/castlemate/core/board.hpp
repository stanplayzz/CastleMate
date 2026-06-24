#pragma once
#include "castlemate/core/piece.hpp"
#include "castlemate/ui/board_view.hpp"

namespace CastleMate {
class App;

class Board {
  public:
	Board(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

  private:
	void load_board();

	std::unique_ptr<BoardView> m_boardView{};

	struct {
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
		std::uint64_t bb[COUNT_]{};

		std::uint64_t white_occ{};
		std::uint64_t black_occ{};
		std::uint64_t occ{};
	} m_position;
};
} // namespace CastleMate