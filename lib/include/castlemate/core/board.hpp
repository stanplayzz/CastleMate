#pragma once
#include "castlemate/core/move.hpp"
#include "castlemate/core/position.hpp"
#include "castlemate/ui/outline.hpp"

namespace CastleMate {
class Board {
  public:
	Board();

	void click_square(glm::ivec2 square, SquareOutline& outline);

	void draw(le::IRenderer& renderer) const;

	[[nodiscard]] auto get_selected_square() const -> std::optional<int> { return m_selected_sq; }

	[[nodiscard]] auto get_bitboard() const -> std::uint64_t const* {
		return static_cast<std::uint64_t const*>(m_position.bb);
	}

	[[nodiscard]] auto should_update_view() -> bool {
		if (m_update_view) {
			m_update_view = false;
			return true;
		}
		return false;
	}

  private:
	void load_board();
	void update_occ();

	void move(Move m);

	Position m_position;

	std::optional<int> m_selected_sq{};
	bool m_update_view{};
};
} // namespace CastleMate