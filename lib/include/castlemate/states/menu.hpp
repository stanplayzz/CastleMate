#pragma once
#include "castlemate/state.hpp"
#include "castlemate/ui/button.hpp"
#include "castlemate/ui/choose_color_menu.hpp"
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/sprite.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/resource/font.hpp>

namespace CastleMate {
class App;

class Menu : public State {
  public:
	Menu(gsl::not_null<App*> app);

	auto update() -> std::unique_ptr<State> override;

	void draw(le::IRenderer& renderer) const override;

	[[nodiscard]] auto get_clear_color() const -> kvf::Color override {
		return Theme::from_name<kvf::Color>({"board", "light"});
	}

  private:
	void create_choose_color_menu();

	gsl::not_null<App*> m_app;

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