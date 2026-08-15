#pragma once
#include <kvf/rect.hpp>
#include <le2d/drawable/text.hpp>
#include <span>
#include <string>

namespace CastleMate::ui {
class KeybindPanel {
  public:
	constexpr static auto keybind_height_v{65};

	explicit KeybindPanel(gsl::not_null<le::IFont*> font, kvf::Rect<float> bounds, std::span<std::string> keybinds) {
		for (std::size_t i = 0; i < keybinds.size(); i++) {
			m_keybinds.emplace_back();
			m_keybinds.back().set_string(*font, keybinds[i],
										 le::drawable::TextParams{.height = le::TextHeight{keybind_height_v},
																  .expand = le::drawable::TextExpand::eRight});
			m_keybinds.back().transform.position = bounds.lt;
			m_keybinds.back().transform.position.y -=
				(m_keybinds.back().get_size().y * 0.5f) + (keybind_height_v * static_cast<float>(i));
		}
	}

	void draw(le::IRenderer& renderer) const {
		for (auto const& keybind : m_keybinds) { keybind.draw(renderer); }
	}

  private:
	std::vector<le::drawable::Text> m_keybinds{};
};
} // namespace CastleMate::ui