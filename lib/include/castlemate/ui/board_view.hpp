#pragma once
#include "castlemate/core/game_ending.hpp"
#include "castlemate/core/piece.hpp"
#include "castlemate/ui/dialog.hpp"
#include "castlemate/ui/outline.hpp"
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/sprite.hpp>
#include <le2d/drawable/text.hpp>

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
		if (m_promotion_dialog) { m_promotion_dialog->open(white); }
	}
	void hide_promotion() {
		if (m_promotion_dialog) { m_promotion_dialog->close(); }
	}

	[[nodiscard]] auto get_square_outline() const -> SquareOutline& { return *m_square_outline; }

	[[nodiscard]] auto get_promotion_ui() -> ui::PromotionDialog& { return *m_promotion_dialog; }

	void end_game(GameEnding ending);

  private:
	void create_board();
	void load_piece_texture();
	void update_pieces(bool white_bottom);

	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_board{};
	std::unique_ptr<le::IShader> m_board_shader{};

	std::unique_ptr<SquareOutline> m_square_outline{};

	std::vector<le::drawable::Sprite> m_piece_sprites{};
	std::array<Piece, 64> m_pieces{};
	std::unique_ptr<le::ITexture> m_piece_texture{};

	std::unique_ptr<ui::PromotionDialog> m_promotion_dialog{};

	std::unique_ptr<le::IFont> m_font{};
	le::drawable::Text m_end_text{};
	le::drawable::Text m_end_sub_text{};
};
} // namespace CastleMate