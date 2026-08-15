#pragma once
#include "castlemate/core/piece.hpp"
#include <le2d/drawable/sprite.hpp>
#include <le2d/resource/texture.hpp>

namespace CastleMate::ui {
class CapturedPieces {
  public:
	explicit CapturedPieces(gsl::not_null<le::ITexture*> texture, kvf::Rect<float> bounds)
		: m_texture(texture), m_bounds(bounds) {}

	void add(Piece piece);

	void draw(le::IRenderer& renderer) const;

  private:
	void order();

	gsl::not_null<le::ITexture*> m_texture;
	kvf::Rect<float> m_bounds{};

	std::vector<std::pair<le::drawable::Sprite, Piece>> m_white_pieces{};
	std::vector<std::pair<le::drawable::Sprite, Piece>> m_black_pieces{};
};
} // namespace CastleMate::ui