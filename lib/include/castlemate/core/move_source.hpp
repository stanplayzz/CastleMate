#pragma once
#include "castlemate/core/move.hpp"
#include "castlemate/network/network_game.hpp"
#include <klib/base_types.hpp>

namespace CastleMate {
class MoveSource : klib::Polymorphic { // NOLINT
  public:
	virtual ~MoveSource() = default;

	[[nodiscard]] virtual auto is_turn() const -> bool = 0;
	[[nodiscard]] virtual auto poll_remote_move() -> std::optional<Move> = 0;
	virtual void send_move(Move move) = 0;
};

class LocalMoveSource final : public MoveSource {
  public:
	[[nodiscard]] auto is_turn() const -> bool override { return true; }
	[[nodiscard]] auto poll_remote_move() -> std::optional<Move> override { return std::nullopt; }
	void send_move(Move /*move*/) override{};
};

class OnlineMoveSource final : public MoveSource {
  public:
	OnlineMoveSource(std::unique_ptr<NetworkGame> game, bool is_white) : m_game(std::move(game)), m_turn(is_white) {}

	[[nodiscard]] auto is_turn() const -> bool override {
		return m_turn && m_game->state() == ConnectionState::Connected;
	}

	[[nodiscard]] auto poll_remote_move() -> std::optional<Move> override {
		auto move = m_game->poll_move();
		if (!move) { return std::nullopt; }
		m_turn = true;
		return move;
	}

	void send_move(Move move) override {
		m_game->send_move(move);
		m_turn = false;
	}

  private:
	std::unique_ptr<NetworkGame> m_game{};
	bool m_turn{};
};
} // namespace CastleMate