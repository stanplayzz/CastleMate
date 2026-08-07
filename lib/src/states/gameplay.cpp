#include "castlemate/states/gameplay.hpp"
#include "castlemate/app.hpp"
#include "castlemate/states/menu.hpp"
#include "castlemate/utils/algebraic.hpp"
#include "castlemate/utils/conversion.hpp"

namespace CastleMate {
Gameplay::Gameplay(gsl::not_null<App*> app, bool white, std::unique_ptr<GameConnection> connection)
	: m_app(app), m_connection(std::move(connection)), m_white_bottom(white) {
	if (m_connection) {
		m_move_source = std::make_unique<OnlineMoveSource>(white);
	} else {
		m_move_source = std::make_unique<LocalMoveSource>();
	}

	m_side_menu = std::make_unique<SideMenu>(app);
	m_board = std::make_unique<Board>(app);
	m_board->set_on_move([this](Move move, Position& pos, bool white) {
		m_side_menu->append_move(to_algebraic(move, pos), white);
		if (white == m_white_bottom) { m_move_source->send_move(move, m_connection.get()); }
	});
	m_board->set_on_capture([this](Piece p) {
		m_side_menu->add_capture(p);
	});
	m_board_view = std::make_unique<BoardView>(app);
	m_board_view->update_board(static_cast<std::uint64_t const*>(m_board->get_bitboard()), m_white_bottom);
}

auto Gameplay::update() -> std::unique_ptr<State> {
	handle_input();

	if (auto move = m_move_source->poll_remote_move(m_connection.get())) { m_board->move(*move); }

	if (m_board->should_update_view()) {
		m_board_view->update_board(static_cast<std::uint64_t const*>(m_board->get_bitboard()), m_white_bottom);
	}

	if (auto white = m_board->show_promotion_view()) {
		m_board_view->show_promotion(*white);
	} else {
		m_board_view->hide_promotion();
	}

	if (auto ending = m_board->get_ending()) { m_board_view->end_game(*ending); }

	if (m_go_main_menu) { return std::make_unique<Menu>(m_app); }
	return nullptr;
}

void Gameplay::draw(le::IRenderer& renderer) const {
	m_board_view->draw(renderer);
	m_side_menu->draw(renderer);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void Gameplay::handle_input() {
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE && !m_board->get_ending()) {
				if (auto white = m_board->show_promotion_view()) {
					auto mouse_world_pos = window_to_world(m_mouse_pos, m_app->get_context().window_size());
					mouse_world_pos.x += (viewport_v.world_size.x - board_size_v.x) * 0.5f;
					for (std::size_t i = 0; i < m_board_view->get_promotion_ui().choices.size(); i++) {
						auto& choice = m_board_view->get_promotion_ui().choices.at(i);
						if (choice.bounding_rect().contains(mouse_world_pos)) {
							auto const pieces = *white ? std::array{WR, WN, WB, WQ} : std::array{BR, BN, BB, BQ};
							m_board->set_promotion(pieces.at(i));
						}
					}
				} else if (m_move_source->is_turn()) {
					auto pos = screen_to_sq(window_to_board(m_mouse_pos, m_app->get_context().window_size()));
					auto sq = static_cast<int>(pos.x + (pos.y * 8));
					sq = m_white_bottom ? sq : 63 - sq;
					if (sq >= 0) { m_board->click_square(sq, m_board_view->get_square_outline(), m_white_bottom); }
				}
			} else {
			}
		}
		if (m_board->get_ending()) {
			if (auto const* key = std::get_if<le::event::Key>(&e)) {
				if (key->action == GLFW_RELEASE) { m_go_main_menu = true; }
			}
		}
		if (auto const* key = std::get_if<le::event::Key>(&e)) {
			if (key->action == GLFW_RELEASE && key->key == GLFW_KEY_F) {
				m_white_bottom = !m_white_bottom;
				m_board_view->update_board(static_cast<std::uint64_t const*>(m_board->get_bitboard()), m_white_bottom);
			}
		}
	}
}

} // namespace CastleMate