#include "castlemate/core/board.hpp"
#include "castlemate/app.hpp"
#include "castlemate/core/movegen.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "castlemate/utils/constants.hpp"

namespace CastleMate {
namespace {
constexpr auto base_fen_v = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

Board::Board(gsl::not_null<App*> app) : m_app(app) {
	m_position = Position::from_fen(base_fen_v);

	m_move_buffer = m_app->create_asset_loader().load<le::IAudioBuffer>("sounds/move.wav");
	m_capture_buffer = m_app->create_asset_loader().load<le::IAudioBuffer>("sounds/capture.wav");
	if (!m_move_buffer || !m_capture_buffer) { throw std::runtime_error{"Failed to load audio buffer"}; }
}

void Board::click_square(std::uint8_t sq, SquareOutline& outline, bool white_bottom) {
	if (m_pending_move) { return; }

	auto display_sq = white_bottom ? sq : 63 - sq;

	if (m_selected_sq.has_value()) {
		move({.from = *m_selected_sq, .to = sq});
		m_selected_sq = std::nullopt;
	} else if (get_bit(m_position.occ, sq)) {
		if ((get_bit(m_position.white_occ, sq) && m_position.turn == Color::Black) ||
			(get_bit(m_position.black_occ, sq) && m_position.turn == Color::White)) {
			return;
		}
		m_selected_sq = sq;
		auto pos = glm::ivec2{display_sq % 8, display_sq / 8};
		outline.set_position((glm::vec2{pos - glm::ivec2{4, 4}} * tile_size_v) + (tile_size_v * 0.5f));
	}
	outline.should_draw = m_selected_sq.has_value();
}

void Board::set_promotion(Piece p) {
	m_pending_move->promotion = p;
	m_should_promote = false;
	finish_move(*m_pending_move);
	m_pending_move = std::nullopt;
}

void Board::update_occ() {
	auto const& bb = m_position.bb;
	m_position.white_occ = bb[WP] | bb[WR] | bb[WN] | bb[WB] | bb[WQ] | bb[WK];
	m_position.black_occ = bb[BP] | bb[BR] | bb[BN] | bb[BB] | bb[BQ] | bb[BK];
	m_position.occ = m_position.white_occ | m_position.black_occ;
}

void Board::move(Move m) {
	auto moves = legal_moves(m_position);
	if (std::ranges::find(moves, m) == moves.end()) { return; }

	// promotion
	auto is_white = get_bit(m_position.bb[WP], m.from);
	auto is_black = get_bit(m_position.bb[BP], m.from);
	if ((is_white && m.to >= 56) || (is_black && m.to < 8)) {
		m_pending_move = m;
		m_should_promote = true;
		return;
	}

	finish_move(m);
}

void Board::finish_move(Move m) {
	auto move_old = m;
	auto pos_old = m_position;

	auto capture = make_move(m_position, m).captured;
	m_white_turn = !m_white_turn;
	m_update_view = true;

	if (in_checkmate(m_position)) {
		m_ending.white_won = !m_white_turn;
		m_has_ended = true;
	}
	if (in_stalemate(m_position)) {
		m_ending.draw = true;
		m_has_ended = true;
	}

	if (!capture) {
		m_app->get_context().get_audio_mixer().play_sfx(m_move_buffer.get());
	} else {
		m_app->get_context().get_audio_mixer().play_sfx(m_capture_buffer.get());
		if (m_on_capture && capture != COUNT_) { m_on_capture(*capture); }
	}

	if (m_on_move) { m_on_move(move_old, pos_old, !m_white_turn); }
}
} // namespace CastleMate