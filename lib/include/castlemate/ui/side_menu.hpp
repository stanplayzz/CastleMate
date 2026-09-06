#pragma once
#include "castlemate/ui/captured_pieces.hpp"
#include "castlemate/ui/keybind_panel.hpp"
#include "castlemate/ui/move_history.hpp"
#include <le2d/drawable/shape.hpp>

namespace CastleMate {
class App;

class SideMenu {
  public:
	SideMenu(gsl::not_null<App const*> app);

	void update_move_list() { m_move_history->update_list(); }

	void append_move(std::string const& notation, bool white);
	void add_capture(Piece piece);

	void draw(le::IRenderer& renderer) const;

  private:
	gsl::not_null<App const*> m_app;

	le::drawable::Quad m_background{};

	std::unique_ptr<le::ITexture> m_piece_texture{};

	std::unique_ptr<ui::MoveHistory> m_move_history{};
	std::unique_ptr<ui::CapturedPieces> m_captured_pieces{};
	std::unique_ptr<ui::KeybindPanel> m_keybind_panel{};

	std::unique_ptr<le::IFont> m_font{};
};
} // namespace CastleMate