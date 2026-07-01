#pragma once
#include "castlemate/ui/button.hpp"
#include "castlemate/ui/widget.hpp"

namespace CastleMate::ui {
struct ChooseColorMenu {
	Widget background{};
	le::drawable::Quad selected{};
	Widget option_background{};
	le::drawable::Text text{};
	SpriteButton white{};
	SpriteButton black{};
	TextButton start{};

	bool white_selected{true};

	void draw(le::IRenderer& renderer) const {
		background.draw(renderer);
		option_background.draw(renderer);
		selected.draw(renderer);
		white.draw(renderer);
		black.draw(renderer);
		text.draw(renderer);
		start.draw(renderer);
	}
};
} // namespace CastleMate::ui