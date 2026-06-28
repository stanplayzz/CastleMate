#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/resource/shader.hpp>

namespace CastleMate {
class App;

class SquareOutline {
  public:
	SquareOutline(gsl::not_null<App const*> app);

	void draw(le::IRenderer& renderer) const;

	void set_position(glm::vec2 pos);

	bool should_draw{false};

  private:
	gsl::not_null<App const*> m_app;

	std::unique_ptr<le::IShader> m_shader{};
	le::drawable::Quad m_quad{};
};
} // namespace CastleMate