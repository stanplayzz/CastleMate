#pragma once
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/renderer.hpp>
#include <deque>
#include <mutex>

namespace CastleMate::ui {
class MoveHistory {
  public:
	explicit MoveHistory(gsl::not_null<le::IFont*> font, kvf::Rect<> const& bounds);

	void append_move(std::string const& algebraic, bool white);

	void update_list();

	void draw(le::IRenderer& renderer) const;

  private:
	struct Row {
		le::drawable::Text number{};
		le::drawable::Text white{};
		le::drawable::Text black{};
		le::drawable::Quad background{};
	};

	struct PendingMove {
		std::string algebraic;
		bool white;
	};

	void add_row(std::string const& notation);

	gsl::not_null<le::IFont*> m_font;
	kvf::Rect<> m_bounds{};
	std::deque<Row> m_rows{};

	int m_move_count{};

	std::mutex m_mutex;
	std::vector<PendingMove> m_pending_moves{};
};
} // namespace CastleMate::ui