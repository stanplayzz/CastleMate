#include "castlemate/states/gameplay.hpp"
#include "castlemate/app.hpp"
#include "castlemate/utils/conversion.hpp"

namespace CastleMate {
Gameplay::Gameplay(gsl::not_null<App const*> app, bool white) : m_app(app), m_white_bottom(white) {
	m_board_view = std::make_unique<BoardView>(app);
	m_board_view->update_board(static_cast<std::uint64_t const*>(m_board.get_bitboard()), m_white_bottom);
}

auto Gameplay::update() -> std::unique_ptr<State> {
	handle_input();

	if (m_board.should_update_view()) {
		m_board_view->update_board(static_cast<std::uint64_t const*>(m_board.get_bitboard()), m_white_bottom);
	}

	if (auto white = m_board.show_promotion_view()) {
		m_board_view->show_promotion(*white);
	} else {
		m_board_view->hide_promotion();
	}

	return nullptr;
}

void Gameplay::draw(le::IRenderer& renderer) const { m_board_view->draw(renderer); }

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void Gameplay::handle_input() {
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE) {
				if (auto white = m_board.show_promotion_view()) {
					for (std::size_t i = 0; i < m_board_view->get_promotion_ui().choices.size(); i++) {
						auto& choice = m_board_view->get_promotion_ui().choices.at(i);
						if (choice.bounding_rect().contains(
								window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
							auto const pieces = *white ? std::array{WR, WN, WB, WQ} : std::array{BR, BN, BB, BQ};
							m_board.set_promotion(pieces.at(i));
						}
					}
				} else {
					auto pos = screen_to_sq(window_to_board(m_mouse_pos, m_app->get_context().window_size()));
					auto sq = static_cast<int>(pos.x + (pos.y * 8));
					sq = m_white_bottom ? sq : 63 - sq;
					m_board.click_square(sq, m_board_view->get_square_outline(), m_white_bottom);
				}
			}
		}
	}
}

} // namespace CastleMate