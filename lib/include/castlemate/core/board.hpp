#pragma once
#include "castlemate/core/game_ending.hpp"
#include "castlemate/core/move.hpp"
#include "castlemate/core/position.hpp"
#include "castlemate/ui/outline.hpp"
#include <le2d/resource/audio_buffer.hpp>

namespace CastleMate {
class Board {
  public:
	Board(gsl::not_null<App*> app);

	void click_square(std::uint8_t sq, SquareOutline& outline, bool white_bottom);

	void set_promotion(Piece p);

	void set_on_move(std::function<void(Move, Position&, std::string const&, bool)> cb) { m_on_move = std::move(cb); }
	void set_on_capture(std::function<void(Piece)> cb) { m_on_capture = std::move(cb); }

	[[nodiscard]] auto get_selected_square() const -> std::optional<int> { return m_selected_sq; }

	[[nodiscard]] auto get_bitboard() const -> std::uint64_t const* {
		return static_cast<std::uint64_t const*>(m_position.bb);
	}

	[[nodiscard]] auto should_update_view() -> bool {
		if (m_update_view) {
			m_update_view = false;
			return true;
		}
		return false;
	}

	[[nodiscard]] auto white_turn() const { return m_position.turn == Color::White; }

	[[nodiscard]] auto show_promotion_view() const -> std::optional<bool> {
		return m_should_promote ? std::optional<bool>{white_turn()} : std::nullopt;
	}

	[[nodiscard]] auto get_ending() const -> std::optional<GameEnding> {
		return m_has_ended ? std::optional<GameEnding>(m_ending) : std::nullopt;
	}

	void set_ending(GameEnding ending) {
		m_has_ended = true;
		m_ending = ending;
	}

	void move(Move m);

  private:
	void update_occ();

	void finish_move(Move m);

	gsl::not_null<App*> m_app;

	Position m_position;

	std::optional<std::uint8_t> m_selected_sq{};
	bool m_update_view{};

	bool m_should_promote{};
	std::optional<Move> m_pending_move{};

	GameEnding m_ending{};
	bool m_has_ended{};

	std::unique_ptr<le::IAudioBuffer> m_move_buffer{};
	std::unique_ptr<le::IAudioBuffer> m_capture_buffer{};

	std::function<void(Move, Position&, std::string const&, bool)> m_on_move{};
	std::function<void(Piece)> m_on_capture;
};
} // namespace CastleMate