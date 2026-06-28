#include "castlemate/states/menu.hpp"
#include "castlemate/app.hpp"
#include "castlemate/states/gameplay.hpp"
#include "castlemate/utils/constants.hpp"
#include "castlemate/utils/conversion.hpp"

namespace CastleMate {
Menu::Menu(gsl::not_null<App const*> app) : m_app(app) {
	m_font = app->create_asset_loader().load<le::IFont>("fonts/Texturina.ttf");
	if (!m_font) { throw std::runtime_error("Failed to load font"); }

	m_play_button = std::make_unique<MenuButton>(*m_font, "PLAY");
	m_quit_button = std::make_unique<MenuButton>(*m_font, "QUIT");
	m_quit_button->move({0, -m_quit_button->border.bounding_rect().size().y * 1.1f});

	auto sprite_size = viewport_v.world_size * 0.2f;
	m_choose_color_menu.background.create(sprite_size * glm::vec2{2, 1});
	m_choose_color_menu.background.tint = Theme::from_name<kvf::Color>({"ui_background"});
	m_choose_color_menu.border.create(sprite_size * glm::vec2{2.1f, 1.1f});
	m_choose_color_menu.border.tint = Theme::from_name<kvf::Color>({"ui_border"});

	constexpr auto cellW = 1.f / 6.f;
	constexpr auto cellH = 1.f / 2.f;
	m_piece_texture = m_app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }
	m_choose_color_menu.white.set_base_size(sprite_size);
	m_choose_color_menu.white.set_texture(m_piece_texture.get());
	m_choose_color_menu.white.set_uv({.lt = {0, 0}, .rb = {cellW, cellH}});
	m_choose_color_menu.white.transform.position.x -= sprite_size.x * 0.5f;
	m_choose_color_menu.black.set_base_size(sprite_size);
	m_choose_color_menu.black.set_texture(m_piece_texture.get());
	m_choose_color_menu.black.set_uv({.lt = {0, cellH}, .rb = {cellW, cellH * 2}});
	m_choose_color_menu.black.transform.position.x += sprite_size.x * 0.5f;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Menu::update() -> std::unique_ptr<State> {
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE) {
				if (m_choose_color) {
					if (m_choose_color_menu.white.bounding_rect().contains(
							window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
						return std::make_unique<Gameplay>(m_app, true);
					}
					if (m_choose_color_menu.black.bounding_rect().contains(
							window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
						return std::make_unique<Gameplay>(m_app, false);
					}
					m_choose_color = false;
					return {};
				}
				if (m_play_button->border.bounding_rect().contains(
						window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
					m_choose_color = true;
				}

				if (m_quit_button->border.bounding_rect().contains(
						window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
					m_app->close();
				}
			}
		}
	}

	return nullptr;
}

void Menu::draw(le::IRenderer& renderer) const {
	m_play_button->border.draw(renderer);
	m_play_button->background.draw(renderer);
	m_play_button->text.draw(renderer);
	m_quit_button->border.draw(renderer);
	m_quit_button->background.draw(renderer);
	m_quit_button->text.draw(renderer);

	if (m_choose_color) {
		m_choose_color_menu.border.draw(renderer);
		m_choose_color_menu.background.draw(renderer);
		m_choose_color_menu.white.draw(renderer);
		m_choose_color_menu.black.draw(renderer);
	}
}
} // namespace CastleMate