#pragma once
#include "castlemate/ui/move_history.hpp"
#include <le2d/drawable/shape.hpp>

namespace CastleMate {
class App;

class SideMenu {
  public:
	SideMenu(gsl::not_null<App const*> app);

	void append_move(std::string const& notation, bool white);

	void draw(le::IRenderer& renderer) const;

  private:
	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_background{};

	std::unique_ptr<ui::MoveHistory> m_move_history;

	std::unique_ptr<le::IFont> m_font{};
};
} // namespace CastleMate