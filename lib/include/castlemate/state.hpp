#pragma once
#include "castlemate/theme.hpp"
#include <le2d/renderer.hpp>

namespace CastleMate {
class State : klib::Polymorphic {
  public:
	virtual auto update() -> std::unique_ptr<State> = 0;
	virtual void draw(le::IRenderer& renderer) const = 0;
	[[nodiscard]] virtual auto get_clear_color() const -> kvf::Color {
		return Theme::from_name<kvf::Color>({"background"});
	}
};

class StateManager {
  public:
	void set_state(std::unique_ptr<State> state) { m_current = std::move(state); }

	void update() {
		if (m_current) {
			if (auto next = m_current->update()) { m_current = std::move(next); }
		}
	}

	void draw(le::IRenderer& renderer) const {
		if (m_current) { m_current->draw(renderer); }
	}

	[[nodiscard]] auto get_clear_color() const -> kvf::Color {
		if (m_current) { return m_current->get_clear_color(); }
		return Theme::from_name<kvf::Color>({"background"});
	}

  private:
	std::unique_ptr<State> m_current{};
};
} // namespace CastleMate