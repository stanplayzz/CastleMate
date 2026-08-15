#pragma once
#include "castlemate/core/move.hpp"
#include "castlemate/network/game_connection.hpp"
#include <klib/base_types.hpp>

namespace CastleMate {
class MoveSource : klib::Polymorphic { // NOLINT
  public:
	virtual ~MoveSource() = default;

	[[nodiscard]] virtual auto is_turn() const -> bool = 0;
	[[nodiscard]] virtual auto poll_remote_move(GameConnection::IncomingEvent event) -> std::optional<Move> = 0;
	virtual void send_move(Move move, GameConnection* conn) = 0;
};

class LocalMoveSource final : public MoveSource {
  public:
	[[nodiscard]] auto is_turn() const -> bool override { return true; }
	[[nodiscard]] auto poll_remote_move(GameConnection::IncomingEvent /*event*/) -> std::optional<Move> override {
		return std::nullopt;
	}
	void send_move(Move /*move*/, GameConnection* /*conn*/) override {};
};

class OnlineMoveSource final : public MoveSource {
  public:
	OnlineMoveSource(bool is_white) : m_turn(is_white) {}

	[[nodiscard]] auto is_turn() const -> bool override { return m_turn; }

	[[nodiscard]] auto poll_remote_move(GameConnection::IncomingEvent event) -> std::optional<Move> override {
		if (event.kind != GameConnection::IncomingEvent::Kind::Move) { return std::nullopt; }
		m_turn = true;
		return event.move;
	}

	void send_move(Move move, GameConnection* conn) override {
		if (!conn) { return; }
		conn->send_move(move);
		m_turn = false;
	}

  private:
	bool m_turn{};
};
} // namespace CastleMate