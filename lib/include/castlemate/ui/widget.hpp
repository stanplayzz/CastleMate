#pragma once
#include "castlemate/theme.hpp"
#include <le2d/drawable/shape.hpp>

namespace CastleMate::ui {
struct Widget : public klib::Polymorphic {
	le::drawable::Quad border{};
	le::drawable::Quad background{};

	Widget() { create(); }

	void create(glm::vec2 size = {100, 100}, float border_size = 8.f) {
		background.create(size);
		border.create(size + (border_size * 2));
		background.tint = Theme::from_name<kvf::Color>({"ui_background"});
		border.tint = Theme::from_name<kvf::Color>({"ui_border"});
		if (border_size <= 0) { border.tint = kvf::Color{glm::vec4{0, 0, 0, 0}}; }
	}

	virtual void draw(le::IRenderer& renderer) const {
		border.draw(renderer);
		background.draw(renderer);
	}

	virtual void set_position(glm::vec2 pos) {
		border.transform.position = pos;
		background.transform.position = pos;
	}

	[[nodiscard]] auto hovered(glm::vec2 mouse_pos) const -> bool { return border.bounding_rect().contains(mouse_pos); }
};
} // namespace CastleMate::ui