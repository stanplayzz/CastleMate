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

	m_play_button.create_sprite({800, 400}, false);
	m_play_button.set_texture(*m_play_texture);
	m_quit_button.set_position({0, -40});
	m_play_button.on_click = [this]() {
		m_choose_mode = true;
	};
	m_quit_button.create_sprite({800, 400}, false);
	m_quit_button.set_texture(*m_quit_texture);
	m_quit_button.set_position({0, -480});
	m_quit_button.on_click = [this]() {
		m_app->close();
	};

	m_logo_texture = m_app->create_asset_loader().load<le::ITexture>("images/castlemate_logo.png");
	if (!m_logo_texture) { throw std::runtime_error{"Failed to load texture"}; }
	m_logo.set_base_size(glm::vec2{1600});
	m_logo.set_texture(m_logo_texture.get());
	m_logo.transform.position.y += 700;

	create_choose_color_menu();
	create_game_mode_menu();
	create_searching_online_menu();
	create_join_lan_menu();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Menu::update() -> std::unique_ptr<State> {
	// INPUT
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		auto world_mouse_pos = window_to_world(m_mouse_pos, m_app->get_context().window_size());
		m_choose_color_menu.start.update(world_mouse_pos);
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE) {
				if (m_choose_mode) {
					m_game_mode_menu.local.click(world_mouse_pos);
					m_game_mode_menu.host_lan.click(world_mouse_pos);
					m_game_mode_menu.join_lan.click(world_mouse_pos);
					m_game_mode_menu.online.click(world_mouse_pos);
					m_game_mode_menu.back.click(world_mouse_pos);
					return {};
				}
				if (m_choose_join_lan) {
					for (auto& button : m_join_lan_menu.host_buttons) { button.click(world_mouse_pos); }
					m_join_lan_menu.back.click(world_mouse_pos);
					return {};
				}
				if (m_searching_online) {
					m_searching_online_menu.cancel.click(world_mouse_pos);
					return {};
				}
				if (m_choose_color) {
					if (m_choose_color_menu.white.hovered(world_mouse_pos) ||
						m_choose_color_menu.black.hovered(world_mouse_pos)) {
						m_choose_color_menu.white_selected = !m_choose_color_menu.white_selected;
						m_choose_color_menu.selected.transform.position =
							m_choose_color_menu.white_selected ? m_choose_color_menu.white.sprite.transform.position
															   : m_choose_color_menu.black.sprite.transform.position;
						m_choose_color_menu.text.set_string(*m_font,
															m_choose_color_menu.white_selected ? "WHITE" : "BLACK",
															{.height = le::TextHeight{100}});
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

	if (m_to_local_game) { return std::make_unique<Gameplay>(m_app, m_choose_color_menu.white_selected, nullptr); }

	if (auto* conn = m_app->network().get_connection(); conn) {
		return std::make_unique<Gameplay>(m_app, true, std::make_unique<GameConnection>(std::move(*conn)));
	}

	if (m_choose_join_lan) { sync_join_lan_menu(); }

	return nullptr;
}

void Menu::draw(le::IRenderer& renderer) const {
	m_logo.draw(renderer);

	if (m_choose_mode) {
		m_game_mode_menu.draw(renderer);
		return;
	}

	if (m_choose_join_lan) {
		m_join_lan_menu.draw(renderer);
		return;
	}

	if (m_searching_online) {
		m_searching_online_menu.draw(renderer);
		return;
	}

	if (m_choose_color) {
		m_choose_color_menu.draw(renderer);
		return;
	}

	m_play_button.draw(renderer);
	m_quit_button.draw(renderer);
}

void Menu::create_choose_color_menu() {
	auto& menu = m_choose_color_menu;
	menu.background.create({1200, 1200}, 8);

	m_piece_texture = m_app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }
	constexpr auto sprite_size = 512.f;
	menu.option_background.create({sprite_size * 2, sprite_size}, 4);
	menu.option_background.set_position({0, 256});
	menu.white.create_sprite({sprite_size, sprite_size}, false);
	menu.white.background.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
	menu.black.create_sprite({sprite_size, sprite_size}, false);
	menu.black.background.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
	menu.white.set_texture(*m_piece_texture, {.lt = {0, 0}, .rb = {1.f / 6.f, 1.f / 2.f}});
	menu.black.set_texture(*m_piece_texture, {.lt = {0, 1.f / 2.f}, .rb = {1.f / 6.f, 1.f}});
	menu.white.set_position({-sprite_size * 0.5f, 256});
	menu.black.set_position({sprite_size * 0.5f, 256});

	menu.selected.create({sprite_size, sprite_size});
	menu.selected.tint = kvf::Color{glm::vec4{0, 0, 0, 0.2f}};
	menu.selected.transform.position = menu.white.sprite.transform.position;

	menu.text.set_string(*m_font, "WHITE", {.height = le::TextHeight{100}});
	menu.text.tint = kvf::black_v;
	menu.text.transform.position = glm::vec2{0, -240};

	menu.start.create({800, 200}, 4);
	menu.start.set_string(*m_font, "PLAY", 120);
	menu.start.text.tint = kvf::black_v;
	menu.start.set_position({0, -400});
	menu.start.on_click = [this]() {
		m_to_local_game = true;
	};
}

void Menu::create_game_mode_menu() {
	auto& menu = m_game_mode_menu;

	menu.background.create({1200, 1200}, 8);

	menu.local.create({800, 150}, 4);
	menu.local.set_string(*m_font, "LOCAL GAME", 90);
	menu.local.text.tint = kvf::black_v;
	menu.local.set_position({0, 400});

	menu.host_lan.create({800, 150}, 4);
	menu.host_lan.set_string(*m_font, "HOST LAN", 90);
	menu.host_lan.text.tint = kvf::black_v;
	menu.host_lan.set_position({0, 200});

	menu.join_lan.create({800, 150}, 4);
	menu.join_lan.set_string(*m_font, "JOIN LAN", 90);
	menu.join_lan.text.tint = kvf::black_v;
	menu.join_lan.set_position({0, 0});

	menu.online.create({800, 150}, 4);
	menu.online.set_string(*m_font, "ONLINE", 90);
	menu.online.text.tint = kvf::black_v;
	menu.online.set_position({0, -200});

	menu.back.create({500, 120}, 4);
	menu.back.set_string(*m_font, "BACK", 80);
	menu.back.text.tint = kvf::black_v;
	menu.back.set_position({0, -500});

	menu.local.on_click = [this]() {
		m_choose_mode = false;
		m_choose_color = true;
	};

	menu.host_lan.on_click = [this]() {
		m_app->network().host_lan();
	};

	menu.join_lan.on_click = [this]() {
		m_choose_mode = false;
		m_choose_join_lan = true;
		m_app->network().browse_lan();
	};

	menu.online.on_click = [this]() {
		m_choose_mode = false;
		m_searching_online = true;
		m_app->network().search_match();
	};

	menu.back.on_click = [this]() {
		m_choose_mode = false;
	};
}

void Menu::create_join_lan_menu() {
	auto& menu = m_join_lan_menu;
	menu.background.create({1200, 1200}, 8);

	menu.back.create({500, 120}, 4);
	menu.back.set_string(*m_font, "BACK", 80);
	menu.back.text.tint = kvf::black_v;
	menu.back.set_position({0, -400});
	menu.back.on_click = [this]() {
		m_choose_join_lan = false;
		m_choose_mode = true;
		m_app->network().stop_browse_lan();
	};
}

void Menu::create_searching_online_menu() {
	auto& menu = m_searching_online_menu;

	menu.background.create({1200, 1200}, 8);

	menu.text.set_string(*m_font, "SEARCHING FOR OPPONENT...", {.height = le::TextHeight{100}});
	menu.text.tint = kvf::black_v;
	menu.text.transform.position = {0, 100};

	menu.cancel.create({500, 120}, 4);
	menu.cancel.set_string(*m_font, "CANCEL", 80);
	menu.cancel.text.tint = kvf::black_v;
	menu.cancel.set_position({0, -300});

	menu.cancel.on_click = [this]() {
		m_searching_online = false;
		m_app->network().cancel_search();
	};
}

void Menu::sync_join_lan_menu() {
	auto const hosts = m_app->network().lan_hosts();
	auto& menu = m_join_lan_menu;

	if (menu.host_buttons.size() == hosts.size()) { return; }

	menu.host_buttons.clear();
	menu.host_buttons.reserve(hosts.size());

	constexpr auto start_y = 300.f;
	constexpr auto spacing = 150.f;

	for (std::size_t i = 0; i < hosts.size(); ++i) {
		auto const& host = hosts[i];
		auto button = ui::TextButton{};
		button.create({800, 120}, 4);
		button.set_string(*m_font, host.host, 70);
		button.text.tint = kvf::black_v;
		button.set_position({0, start_y - (static_cast<float>(i) * spacing)});
		button.on_click = [this, host]() {
			m_app->network().join_lan(host);
		};
		menu.host_buttons.push_back(std::move(button));
	}
}

} // namespace CastleMate