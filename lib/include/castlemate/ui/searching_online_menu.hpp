#pragma once
#include "castlemate/ui/button.hpp"

namespace CastleMate::ui {
struct SearchingOnlineMenu {
	ui::Widget background{};
	le::drawable::Text text{};
	ui::TextButton cancel{};

	void draw(le::IRenderer& renderer) const {
		background.draw(renderer);
		text.draw(renderer);
		cancel.draw(renderer);
	}
};
} // namespace CastleMate::ui