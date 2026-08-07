#pragma once
#include "castlemate/core/move.hpp"
#include <bnet/connection.hpp>
#include <mutex>
#include <queue>

namespace CastleMate {
enum class ConnectionState : std::uint8_t {
	Connected,
	Disconnected,
	TimedOut,
};

class GameConnection { // NOLINT
  public:
	explicit GameConnection(bnet::Connection connection);

	void send_move(Move move);

	[[nodiscard]] auto poll_move() -> std::optional<Move>;

	[[nodiscard]] auto state() { return m_state.load(); }

  private:
	void receive_loop(std::stop_token const& token);

	bnet::Connection m_connection;
	std::jthread m_recv_thread{};
	std::atomic<ConnectionState> m_state{ConnectionState::Connected};

	std::mutex m_queue_mutex{};
	std::queue<Move> m_incoming{};
};
} // namespace CastleMate