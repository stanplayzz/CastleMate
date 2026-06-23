#pragma once
#include <le2d/drawable/shape.hpp>

namespace CastleMate {
class App;

class BoardView {
  public:
	BoardView(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

  private:
	void create_board();

	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_board{};
	std::unique_ptr<le::IShader> m_board_shader{};
};
} // namespace CastleMate