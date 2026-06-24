#pragma once
#include "castlemate/core/piece.hpp"
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/sprite.hpp>

namespace CastleMate {
class App;

class BoardView {
  public:
	BoardView(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
	void update_board(std::uint64_t const* bitboards);

  private:
	void create_board();
	void load_piece_texture();
	void update_pieces();

	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_board{};
	std::unique_ptr<le::IShader> m_board_shader{};

	std::vector<le::drawable::Sprite> m_piece_sprites{};
	std::array<Piece, 64> m_pieces{};
	std::unique_ptr<le::ITexture> m_piece_texture{};
};
} // namespace CastleMate