#include "castlemate/ui/side_menu.hpp"
#include "castlemate/app.hpp"
#include "castlemate/theme.hpp"
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

	m_move_history = std::make_unique<ui::MoveHistory>(m_font.get(), m_background.bounding_rect());
	auto rect = m_background.bounding_rect();
	rect.lt.y = 0;
	m_captured_pieces = std::make_unique<ui::CapturedPieces>(m_piece_texture.get(), rect);
}

void SideMenu::append_move(std::string const& notation, bool white) { m_move_history->append_move(notation, white); }

void SideMenu::add_capture(Piece piece) { m_captured_pieces->add(piece); }

void SideMenu::draw(le::IRenderer& renderer) const {
	m_background.draw(renderer);
	m_move_history->draw(renderer);
	m_captured_pieces->draw(renderer);
}
} // namespace CastleMate