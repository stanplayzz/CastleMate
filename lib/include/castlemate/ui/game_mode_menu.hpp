#pragma once
#include "castlemate/ui/button.hpp"
#include "castlemate/ui/widget.hpp"

namespace CastleMate::ui {
struct GameModeMenu {
	Widget background{};

	TextButton local{};
	TextButton online{};
	TextButton back{};

	void draw(le::IRenderer& renderer) const {
		background.draw(renderer);
		local.draw(renderer);
		online.draw(renderer);
		back.draw(renderer);
	}
};
} // namespace CastleMate::ui