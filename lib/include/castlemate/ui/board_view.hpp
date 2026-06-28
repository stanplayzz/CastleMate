#pragma once
#include "castlemate/core/piece.hpp"
#include "castlemate/ui/outline.hpp"
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/sprite.hpp>

namespace CastleMate {
class App;

struct PromotionUI {
	le::drawable::Quad background{};
	le::drawable::Quad border{};
	std::array<le::drawable::Sprite, 4> choices{};
};

class BoardView {
  public:
	BoardView(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
	void update_board(std::uint64_t const* bitboards, bool white_bottom);

	void show_promotion(bool white) {
		if (!m_show_promotion) {
			m_show_promotion = true;
			create_promotion(white);
		}
	}
	void hide_promotion() { m_show_promotion = false; }

	[[nodiscard]] auto get_square_outline() const -> SquareOutline& { return *m_square_outline; }

	[[nodiscard]] auto get_promotion_ui() -> PromotionUI& { return m_promotion_ui; }

  private:
	void create_board();
	void load_piece_texture();
	void update_pieces(bool white_bottom);
	void create_promotion(bool white);

	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_board{};
	std::unique_ptr<le::IShader> m_board_shader{};

	std::unique_ptr<SquareOutline> m_square_outline{};

	std::vector<le::drawable::Sprite> m_piece_sprites{};
	std::array<Piece, 64> m_pieces{};
	std::unique_ptr<le::ITexture> m_piece_texture{};

	bool m_show_promotion{};
	PromotionUI m_promotion_ui{};
};
} // namespace CastleMate