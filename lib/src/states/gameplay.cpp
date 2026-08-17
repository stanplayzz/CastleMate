#include "castlemate/states/gameplay.hpp"
#include "castlemate/app.hpp"
#include "castlemate/states/menu.hpp"
#include "castlemate/utils/conversion.hpp"

namespace CastleMate {
Gameplay::Gameplay(gsl::not_null<App*> app, bool white)
	: m_app(app), m_white_bottom(white), m_color(white ? Color::White : Color::Black) {
	if (m_app->network().matched_game()) {
		m_connection = std::make_unique<GameConnection>(m_app->network().get_connection());
	} else {
		m_engine =
			std::make_unique<UciEngine>("/home/stan/projects/castlemate/out/clang/engine/Debug/castlemate-engine");

		m_engine->on_best_move([this](Move m) {
			m_board->move(m);
		});
		m_engine->new_game();
	}

	m_font = app->create_asset_loader().load<le::IFont>("fonts/CormorantGaramond.ttf");
	if (!m_font) { throw std::runtime_error{"Failed to load font"}; }

	m_side_menu = std::make_unique<SideMenu>(app);
	m_board = std::make_unique<Board>(app);
	m_board->set_on_move([this](Move move, Position& pos, std::string const& algebraic, bool white) {
		m_side_menu->append_move(algebraic, white);

		// !is_turn() because the turn is already over
		// when this callback is called
		if (m_connection && !is_turn()) { m_connection->send_move(move); }

		if (m_engine && !is_turn()) {
			m_engine->set_position(pos);
			m_engine->go();
		}
	});
	m_board->set_on_capture([this](Piece p) {
		m_side_menu->add_capture(p);
	});
	m_board_view = std::make_unique<BoardView>(app);
	m_board_view->update_board(static_cast<std::uint64_t const*>(m_board->get_bitboard()), m_white_bottom);

	m_confirm_dialog = std::make_unique<ui::ConfirmDialog>(m_app, *m_font);
}

auto Gameplay::update() -> std::unique_ptr<State> {
	handle_input();

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

	m_side_menu->update_move_list();

	if (m_connection) {
		auto conn_event = m_connection->poll_event();
		if (!conn_event) { return nullptr; }

		if (conn_event->kind != GameConnection::IncomingEvent::Kind::Move) { m_board->move(conn_event->move); }
		if (conn_event->kind == GameConnection::IncomingEvent::Kind::DrawOffer) {
			m_pending_confirm = PendingConfirm::DrawAccept;
			m_confirm_dialog->open("Opponent requested a draw, accept?");
		}
		if (conn_event->kind == GameConnection::IncomingEvent::Kind::GameOver) {
			if (conn_event->game_over.reason == shared::GameOverReason::Resign) {
				m_board->set_ending({.resign = true, .white_won = !m_board->white_turn()});
			}
			if (conn_event->game_over.reason == shared::GameOverReason::Draw) { m_board->set_ending({.draw = true}); }
		}
	}

	return nullptr;
}

void Gameplay::draw(le::IRenderer& renderer) const {
	m_board_view->draw(renderer);
	m_side_menu->draw(renderer);

	if (m_confirm_dialog->is_open()) { m_confirm_dialog->draw(renderer); }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void Gameplay::handle_input() {
	for (auto const& e : m_app->get_context().event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 && mouse->action == GLFW_RELEASE && !m_board->get_ending()) {
				auto mouse_world_pos = window_to_world(m_mouse_pos, m_app->get_context().window_size());
				mouse_world_pos.x += (viewport_v.world_size.x - board_size_v.x) * 0.5f;
				if (auto white = m_board->show_promotion_view()) {

					if (auto piece = m_board_view->get_promotion_ui().click(mouse_world_pos)) {
						if (piece) {
							m_board->set_promotion(*piece);
							m_board_view->hide_promotion();
						}
					}
				} else if (m_confirm_dialog->is_open() && m_pending_confirm != PendingConfirm::None) {
					if (auto val =
							m_confirm_dialog->click(window_to_world(m_mouse_pos, m_app->get_context().window_size()))) {
						if (*val) {
							if (m_pending_confirm == PendingConfirm::Resign) {
								if (m_connection) { m_connection->resign(); }
								m_board->set_ending({.resign = true, .white_won = !m_board->white_turn()});
							} else if (m_pending_confirm == PendingConfirm::Draw) {
								if (m_connection) { m_connection->offer_draw(); }
							} else if (m_pending_confirm == PendingConfirm::DrawAccept) {
								if (m_connection) {
									m_connection->accept_draw();
									m_board->set_ending({.draw = true});
								}
							}
						}
						m_pending_confirm = PendingConfirm::None;
						m_confirm_dialog->close();
					}
				} else if (is_turn()) {
					auto pos = screen_to_sq(window_to_board(m_mouse_pos, m_app->get_context().window_size()));
					auto sq = pos.x + (pos.y * 8);
					if (sq >= 0 && sq < 64) {
						sq = m_white_bottom ? sq : 63 - sq;
						m_board->click_square(static_cast<std::uint8_t>(sq), m_board_view->get_square_outline(),
											  m_white_bottom);
					}
				}
			}
		}
		if (m_board->get_ending()) {
			if (auto const* key = std::get_if<le::event::Key>(&e)) {
				if (key->action == GLFW_RELEASE) {
					m_go_main_menu = true;
					if (m_engine) { m_engine->stop(); }
				}
			}
		}
		if (auto const* key = std::get_if<le::event::Key>(&e)) {
			if (key->action == GLFW_RELEASE && key->key == GLFW_KEY_F) {
				m_white_bottom = !m_white_bottom;
				m_board_view->update_board(static_cast<std::uint64_t const*>(m_board->get_bitboard()), m_white_bottom);
			}
			if (key->action == GLFW_RELEASE && key->key == GLFW_KEY_G) {
				if (is_turn() && !m_confirm_dialog->is_open()) {
					m_pending_confirm = PendingConfirm::Draw;
					m_confirm_dialog->open("Are you sure you want to draw?");
				}
			}
			if (key->action == GLFW_RELEASE && key->key == GLFW_KEY_H) {
				if (is_turn() && !m_confirm_dialog->is_open()) {
					m_pending_confirm = PendingConfirm::Resign;
					m_confirm_dialog->open("Are you sure you want to resign?");
				}
			}
		}
	}
}

} // namespace CastleMate