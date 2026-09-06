#pragma once
#include "castlemate/core/board.hpp"
#include "castlemate/engine/engine.hpp"
#include "castlemate/network/game_connection.hpp"
#include "castlemate/state.hpp"
#include "castlemate/ui/board_view.hpp"
#include "castlemate/ui/side_menu.hpp"

namespace CastleMate {
class App;

enum class PendingConfirm : std::uint8_t { None, Resign, Draw, DrawAccept };

class Gameplay : public State {
  public:
	Gameplay(gsl::not_null<App*> app, bool white);

	auto update() -> std::unique_ptr<State> override;
	void draw(le::IRenderer& renderer) const override;

  private:
	void handle_input();
	[[nodiscard]] auto is_turn() const -> bool { return (m_color == Color::White) == m_board->white_turn(); }

	glm::vec2 m_mouse_pos{};

	gsl::not_null<App*> m_app;
	std::unique_ptr<le::IFont> m_font{};

	std::unique_ptr<GameConnection> m_connection{};

	std::unique_ptr<Board> m_board{};
	std::unique_ptr<BoardView> m_board_view{};
	std::unique_ptr<SideMenu> m_side_menu{};

	std::unique_ptr<ui::ConfirmDialog> m_confirm_dialog{};
	PendingConfirm m_pending_confirm{};

	IEngine* m_engine{};

	bool m_white_bottom{};

	bool m_go_main_menu{};

	Color m_color{};
};
} // namespace CastleMate