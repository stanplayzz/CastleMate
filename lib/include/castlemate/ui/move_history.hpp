#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/renderer.hpp>
#include <deque>

namespace CastleMate::ui {
class MoveHistory {
  public:
	explicit MoveHistory(gsl::not_null<le::IFont*> font, kvf::Rect<> const& bounds);

	void append_move(std::string const& notation, bool white);
	void draw(le::IRenderer& renderer) const;

  private:
	struct Row {
		le::drawable::Text number{};
		le::drawable::Text white{};
		le::drawable::Text black{};
		le::drawable::Quad background{};
	};

	void add_row(std::string const& notation);

	gsl::not_null<le::IFont*> m_font;
	kvf::Rect<> m_bounds{};
	std::vector<std::pair<std::string, std::string>> m_moves{};
	std::deque<Row> m_rows{};
	int m_move_count{};
};
} // namespace CastleMate::ui