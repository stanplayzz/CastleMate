#include "castlemate/ui/move_history.hpp"
#include "castlemate/theme.hpp"

namespace CastleMate::ui {
namespace {
constexpr auto max_moves_v = 8;
constexpr auto row_height_v = 80;
constexpr auto number_col_width_v = 70;
constexpr auto move_col_width_v = 220;
} // namespace

MoveHistory::MoveHistory(gsl::not_null<le::IFont*> font, kvf::Rect<> const& bounds) : m_font(font), m_bounds(bounds) {}

void MoveHistory::append_move(std::string const& algebraic, bool white) {
	std::lock_guard lock{m_mutex};
	m_pending_moves.push_back({.algebraic = algebraic, .white = white});
}

void MoveHistory::update_list() {
	auto moves = std::vector<PendingMove>{};
	{
		std::lock_guard lock(m_mutex);
		moves.swap(m_pending_moves);
	}
	for (auto& move : moves) {
		if (move.white) {
			add_row(move.algebraic);
		} else if (!m_rows.empty()) {
			m_rows.back().black.set_string(*m_font, move.algebraic, {.height = le::TextHeight{60}});
		}
	}
}

void MoveHistory::draw(le::IRenderer& renderer) const {
	for (auto const& row : m_rows) {
		row.background.draw(renderer);
		row.number.draw(renderer);
		row.white.draw(renderer);
		row.black.draw(renderer);
	}
}

void MoveHistory::add_row(std::string const& notation) {
	if (m_rows.size() == max_moves_v) {
		m_rows.pop_front();
		for (auto& row : m_rows) {
			row.background.transform.position.y += row_height_v;
			row.number.transform.position.y += row_height_v;
			row.white.transform.position.y += row_height_v;
			row.black.transform.position.y += row_height_v;
		}
	}

	auto row = Row{};
	auto const y = m_bounds.top_left().y - (static_cast<float>(m_rows.size()) * row_height_v) - (row_height_v * 0.5f);

	row.background.create({m_bounds.size().x, row_height_v});
	row.background.transform.position = {m_bounds.top_left().x + (m_bounds.size().x * 0.5f), y};
	row.background.tint = ((m_move_count % 2) == 0) ? Theme::from_name<kvf::Color>({"side_menu", "row_even"})
													: Theme::from_name<kvf::Color>({"side_menu", "row_odd"});

	row.number.set_string(*m_font, std::to_string(m_move_count + 1) + ".",
						  {.height = le::TextHeight{50}, .expand = le::drawable::TextExpand::eRight});
	row.number.transform.position = {m_bounds.top_left().x + 8.0f, y};
	row.number.tint = Theme::from_name<kvf::Color>({"side_menu", "move_number"});

	row.white.set_string(*m_font, notation, {.height = le::TextHeight{55}, .expand = le::drawable::TextExpand::eRight});
	row.white.transform.position = {m_bounds.top_left().x + number_col_width_v, y};
	row.black.set_string(*m_font, "", {.height = le::TextHeight{55}, .expand = le::drawable::TextExpand::eRight});
	row.black.transform.position = {m_bounds.top_left().x + number_col_width_v + move_col_width_v, y};

	++m_move_count;

	m_rows.push_back(std::move(row));
}
} // namespace CastleMate::ui