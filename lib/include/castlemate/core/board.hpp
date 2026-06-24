#pragma once
#include "castlemate/core/piece.hpp"
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

  private:
	void load_board();
	void update_occ();

	struct {
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
		std::uint64_t bb[COUNT_]{};

		std::uint64_t white_occ{};
		std::uint64_t black_occ{};
		std::uint64_t occ{};
	} m_position;

	std::optional<int> m_selected_sq{};
};
} // namespace CastleMate