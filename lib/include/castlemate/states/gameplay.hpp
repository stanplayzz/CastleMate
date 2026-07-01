#pragma once
#include "castlemate/core/board.hpp"
#include "castlemate/state.hpp"
#include "castlemate/ui/board_view.hpp"

namespace CastleMate {
class App;

class Gameplay : public State {
  public:
	Gameplay(gsl::not_null<App const*> app, bool white);

	auto update() -> std::unique_ptr<State> override;
	void draw(le::IRenderer& renderer) const override;

  private:
	void handle_input();
	glm::vec2 m_mouse_pos{};

	gsl::not_null<App const*> m_app;

	Board m_board{};
	std::unique_ptr<BoardView> m_board_view{};

	bool m_white_bottom{};

	bool m_go_main_menu{};
};
} // namespace CastleMate