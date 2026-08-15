#pragma once
#include <kvf/rect.hpp>

namespace CastleMate::ui {
class VerticalStack {
  public:
	explicit VerticalStack(kvf::Rect<float> bounds, float padding = 8.f)
		: m_next_y(bounds.lt.y), m_bounds(bounds), m_padding(padding) {}

	auto take(float height) -> kvf::Rect<float> {
		auto ret = m_bounds;
		ret.lt.y = m_next_y;
		ret.rb.y = m_next_y - height;
		m_next_y -= (height + m_padding);
		return ret;
	}

  private:
	float m_next_y{};
	kvf::Rect<float> m_bounds{};
	float m_padding{};
};
} // namespace CastleMate::ui