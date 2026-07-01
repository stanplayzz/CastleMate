#pragma once
#include "castlemate/state.hpp"
#include "castlemate/ui/button.hpp"
#include "castlemate/ui/choose_color_menu.hpp"
#include "castlemate/utils/constants.hpp"
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/sprite.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/resource/font.hpp>

namespace CastleMate {
class App;

struct MenuButton {
	le::drawable::Quad border{};
	le::drawable::Quad background{};
	le::drawable::Text text{};

	MenuButton(le::IFont& font, std::string const& text) {
		background.create({viewport_v.world_size.x * 0.7f, viewport_v.world_size.x * 0.14f});
		border.create(background.get_size() + glm::vec2{background.get_size().x * 0.03f});
		background.tint = Theme::from_name<kvf::Color>({"ui_background"});
		border.tint = Theme::from_name<kvf::Color>({"ui_border"});

		this->text.set_string(font, text, {.height = le::TextHeight{60}});
		this->text.transform.position.y -= this->text.get_size().y * 0.5f;
	}

	void move(glm::vec2 pos) {
		border.transform.position += pos;
		background.transform.position += pos;
		text.transform.position += pos;
	}
};

class Menu : public State {
  public:
	Menu(gsl::not_null<App const*> app);

	auto update() -> std::unique_ptr<State> override;

	void draw(le::IRenderer& renderer) const override;

	[[nodiscard]] auto get_clear_color() const -> kvf::Color override {
		return Theme::from_name<kvf::Color>({"board", "light"});
	}

  private:
	void create_choose_color_menu();

	gsl::not_null<App const*> m_app;

	glm::vec2 m_mouse_pos{};

	std::unique_ptr<le::IFont> m_font{};

	std::unique_ptr<le::ITexture> m_play_texture{};
	std::unique_ptr<le::ITexture> m_quit_texture{};
	ui::SpriteButton m_play_button{};
	ui::SpriteButton m_quit_button{};

	ui::ChooseColorMenu m_choose_color_menu{};
	bool m_choose_color{};
	std::unique_ptr<le::ITexture> m_piece_texture{};
	std::unique_ptr<le::ITexture> m_logo_texture{};
	le::drawable::Sprite m_logo{};

	bool m_to_game{};
};
} // namespace CastleMate