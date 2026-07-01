#include "castlemate/states/menu.hpp"
#include "castlemate/app.hpp"
#include "castlemate/states/gameplay.hpp"
#include "castlemate/utils/conversion.hpp"

namespace CastleMate {
Menu::Menu(gsl::not_null<App*> app) : m_app(app) {
	queen_move(0, 0, 0);

	m_font = app->create_asset_loader().load<le::IFont>("fonts/CormorantGaramond.ttf");
	if (!m_font) { throw std::runtime_error("Failed to load font"); }

	m_play_texture = app->create_asset_loader().load<le::ITexture>("images/play_button.png");
	m_quit_texture = app->create_asset_loader().load<le::ITexture>("images/quit_button.png");
	if (!m_play_texture || !m_quit_texture) { throw std::runtime_error{"Failed to load texture"}; }

	m_play_button.create_sprite({200, 100}, false);
	m_play_button.set_texture(*m_play_texture);
	m_quit_button.set_position({0, -10});
	m_play_button.on_click = [this]() {
		m_choose_color = true;
	};
	m_quit_button.create_sprite({200, 100}, false);
	m_quit_button.set_texture(*m_quit_texture);
	m_quit_button.set_position({0, -120});
	m_quit_button.on_click = [this]() {
		m_app->close();
	};

	m_logo_texture = m_app->create_asset_loader().load<le::ITexture>("images/castlemate_logo.png");
	if (!m_logo_texture) { throw std::runtime_error{"Failed to load texture"}; }
	m_logo.set_base_size(glm::vec2{400});
	m_logo.set_texture(m_logo_texture.get());
	m_logo.transform.position.y += 150;

	create_choose_color_menu();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Menu::update() -> std::unique_ptr<State> {
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		auto world_mouse_pos = window_to_world(m_mouse_pos, m_app->get_context().window_size());
		m_choose_color_menu.start.update(world_mouse_pos);
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE) {
				if (m_choose_color) {
					if (m_choose_color_menu.white.hovered(world_mouse_pos) ||
						m_choose_color_menu.black.hovered(world_mouse_pos)) {
						m_choose_color_menu.white_selected = !m_choose_color_menu.white_selected;
						m_choose_color_menu.selected.transform.position =
							m_choose_color_menu.white_selected ? m_choose_color_menu.white.sprite.transform.position
															   : m_choose_color_menu.black.sprite.transform.position;
						m_choose_color_menu.text.set_string(*m_font,
															m_choose_color_menu.white_selected ? "WHITE" : "BLACK",
															{.height = le::TextHeight{20}});
						return {};
					}
					if (m_choose_color_menu.start.hovered(world_mouse_pos)) {
						m_choose_color_menu.start.click(world_mouse_pos);
						return {};
					}
					if (!m_choose_color_menu.background.hovered(world_mouse_pos)) { m_choose_color = false; }
					return {};
				}
				m_play_button.click(world_mouse_pos);
				m_quit_button.click(world_mouse_pos);
			}
		}
	}

	if (m_to_game) { return std::make_unique<Gameplay>(m_app, m_choose_color_menu.white_selected); }

	return nullptr;
}

void Menu::draw(le::IRenderer& renderer) const {
	m_logo.draw(renderer);
	if (m_choose_color) {
		m_choose_color_menu.draw(renderer);
		return;
	}

	m_play_button.draw(renderer);
	m_quit_button.draw(renderer);
}

void Menu::create_choose_color_menu() {
	auto& menu = m_choose_color_menu;
	menu.background.create({300, 300}, 2);

	m_piece_texture = m_app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }
	constexpr auto sprite_size = 128.f;
	menu.option_background.create({sprite_size * 2, sprite_size}, 1);
	menu.option_background.set_position({0, 64});
	menu.white.create_sprite({sprite_size, sprite_size}, 0);
	menu.white.background.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
	menu.black.create_sprite({sprite_size, sprite_size}, 0);
	menu.black.background.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
	menu.white.set_texture(*m_piece_texture, {.lt = {0, 0}, .rb = {1.f / 6.f, 1.f / 2.f}});
	menu.black.set_texture(*m_piece_texture, {.lt = {0, 1.f / 2.f}, .rb = {1.f / 6.f, 1.f}});
	menu.white.set_position({-sprite_size * 0.5f, 64});
	menu.black.set_position({sprite_size * 0.5f, 64});

	menu.selected.create({sprite_size, sprite_size});
	menu.selected.tint = kvf::Color{glm::vec4{0, 0, 0, 0.2f}};
	menu.selected.transform.position = menu.white.sprite.transform.position;

	menu.text.set_string(*m_font, "WHITE", {.height = le::TextHeight{20}});
	menu.text.tint = kvf::black_v;
	menu.text.transform.position = glm::vec2{0, -60};

	menu.start.create({200, 50}, 1);
	menu.start.set_string(*m_font, "PLAY");
	menu.start.text.tint = kvf::black_v;
	menu.start.set_position({0, -100});
	menu.start.on_click = [this]() {
		m_to_game = true;
	};
}
} // namespace CastleMate