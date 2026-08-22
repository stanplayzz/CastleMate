#include "castlemate/core/board.hpp"
#include "castlemate/app.hpp"
#include "castlemate/core/movegen.hpp"
#include "castlemate/utils/algebraic.hpp"
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

void Board::click_square(std::uint8_t sq, SquareOutline& outline, bool white_bottom, Color player) {
	if (m_pending_move) { return; }

	auto own_piece = [&](std::uint8_t s) {
		return (get_bit(m_position.white_occ, s) && player == Color::White) ||
			   (get_bit(m_position.black_occ, s) && player == Color::Black);
	};

	auto select = [&](std::uint8_t s) {
		m_selected_sq = s;
		auto display_sq = white_bottom ? s : 63 - s;
		auto pos = glm::ivec2{display_sq % 8, display_sq / 8};
		outline.set_position((glm::vec2{pos - glm::ivec2{4, 4}} * tile_size_v) + (tile_size_v * 0.5f));
	};

	if (m_selected_sq.has_value()) {
		if (own_piece(sq)) {
			select(sq);
		} else {
			if (player == m_position.turn) { move({.from = *m_selected_sq, .to = sq}); }
			m_selected_sq = std::nullopt;
		}
	} else if (get_bit(m_position.occ, sq) && own_piece(sq)) {
		select(sq);
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

	auto const is_white = get_bit(m_position.bb[WP], m.from);
	auto const is_black = get_bit(m_position.bb[BP], m.from);
	auto const is_promotion = (is_white && m.to >= 56) || (is_black && m.to < 8);

	if (is_promotion) {
		// reduce matches to only .from and .to so UI clicks work
		auto matches = [&](Move const& legal) {
			return legal.from == m.from && legal.to == m.to;
		};
		if (std::ranges::find_if(moves, matches) == moves.end()) { return; }

		m_pending_move = m;
		m_should_promote = true;
		return;
	}

	if (std::ranges::find(moves, m) == moves.end()) { return; }

	finish_move(m);
}

void Board::finish_move(Move m) {
	auto white = m_position.turn == Color::White;
	auto algebraic = to_algebraic(m, m_position);

	auto capture = make_move(m_position, m).captured;
	m_update_view = true;

	if (in_checkmate(m_position)) {
		m_ending.white_won = m_position.turn != Color::White;
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

	if (m_on_move) { m_on_move(m, m_position, algebraic, white); }
}
} // namespace CastleMate