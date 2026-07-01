#pragma once
#include "castlemate/ui/widget.hpp"
#include <le2d/drawable/sprite.hpp>
#include <le2d/drawable/text.hpp>

namespace CastleMate::ui {
struct TextButton : Widget {
	std::function<void()> on_click{};
	le::drawable::Text text{};

	void update(glm::vec2 mouse_pos) {
		if (hovered(mouse_pos)) {
			background.tint = Theme::from_name<kvf::Color>({"ui_hover"});
			return;
		}
		background.tint = Theme::from_name<kvf::Color>({"ui_background"});
	}

	void click(glm::vec2 mouse_pos) {
		if (hovered(mouse_pos)) { on_click(); }
	}

	void draw(le::IRenderer& renderer) const override {
		border.draw(renderer);
		background.draw(renderer);
		text.draw(renderer);
	}

	void set_position(glm::vec2 pos) override {
		border.transform.position = pos;
		background.transform.position = pos;
		text.transform.position = background.transform.position - glm::vec2{0, text.get_size().y * 0.5f};
	}

	void set_string(le::IFont& font, std::string const& string) {
		text.set_string(font, string);
		set_position(background.transform.position);
	}
};

struct SpriteButton : Widget {
	std::function<void()> on_click{};
	le::drawable::Sprite sprite{};

	void create_sprite(glm::vec2 size, bool draw_bg) {
		create(size);
		sprite.set_base_size(size);
		if (!draw_bg) {
			background.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
			border.tint = kvf::Color{glm::vec4{0, 0, 0, 0}};
		}
	}

	void update(glm::vec2 mouse_pos) {
		if (hovered(mouse_pos)) {
			background.tint = Theme::from_name<kvf::Color>({"ui_hover"});
			return;
		}
		background.tint = Theme::from_name<kvf::Color>({"ui_background"});
	}

	void click(glm::vec2 mouse_pos) {
		if (hovered(mouse_pos)) { on_click(); }
	}

	void draw(le::IRenderer& renderer) const override {
		border.draw(renderer);
		background.draw(renderer);
		sprite.draw(renderer);
	}

	void set_position(glm::vec2 pos) override {
		border.transform.position = pos;
		background.transform.position = pos;
		sprite.transform.position = pos;
	}

	void set_texture(le::ITexture& texture, kvf::UvRect uv = kvf::uv_rect_v) { sprite.set_texture(&texture, uv); }
};
} // namespace CastleMate::ui