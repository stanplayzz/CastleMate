#pragma once
#include "castlemate/ui/button.hpp"
#include "castlemate/ui/widget.hpp"
#include <vector>

namespace CastleMate::ui {
struct JoinLanMenu {
	Widget background{};
	std::vector<TextButton> host_buttons{};
	TextButton back{};

	void draw(le::IRenderer& renderer) const {
		background.draw(renderer);
		for (auto const& button : host_buttons) { button.draw(renderer); }
		back.draw(renderer);
	}
};
} // namespace CastleMate::ui