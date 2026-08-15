#include "castlemate/ui/side_menu.hpp"
#include "castlemate/app.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/ui/layout.hpp"
#include "castlemate/utils/constants.hpp"

namespace CastleMate {

SideMenu::SideMenu(gsl::not_null<App const*> app) : m_app(app) {
	m_font = app->create_asset_loader().load<le::IFont>("fonts/CormorantGaramond.ttf");
	if (!m_font) { throw std::runtime_error{"Failed to load font"}; }
	m_piece_texture = app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }

	auto const x_size = viewport_v.world_size.x - board_size_v.x;
	m_background.create({x_size, viewport_v.world_size.y});
	m_background.transform.position = {(viewport_v.world_size.x * 0.5f) - (x_size * 0.5f), 0};
	m_background.tint = Theme::from_name<kvf::Color>({"side_menu", "background"});

	auto stack = ui::VerticalStack{m_background.bounding_rect()};

	m_move_history = std::make_unique<ui::MoveHistory>(m_font.get(), stack.take(640.f));

	auto height = (m_background.bounding_rect().size().x / 5.f) * 6.f;
	m_captured_pieces = std::make_unique<ui::CapturedPieces>(m_piece_texture.get(), stack.take(height));

	auto keybinds = std::array<std::string, 3>{
		"  F - Flip Board",
		"  G - Offer Draw",
		"  H - Resign",
	};

	m_keybind_panel = std::make_unique<ui::KeybindPanel>(
		m_font.get(), stack.take(ui::KeybindPanel::keybind_height_v * keybinds.size()), keybinds);
}

void SideMenu::append_move(std::string const& notation, bool white) { m_move_history->append_move(notation, white); }

void SideMenu::add_capture(Piece piece) { m_captured_pieces->add(piece); }

void SideMenu::draw(le::IRenderer& renderer) const {
	m_background.draw(renderer);
	m_move_history->draw(renderer);
	m_captured_pieces->draw(renderer);
	m_keybind_panel->draw(renderer);
}
} // namespace CastleMate