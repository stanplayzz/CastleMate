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

class NetworkGame { // NOLINT
  public:
	explicit NetworkGame(bnet::Connection connection);
	~NetworkGame();

	void send_move(Move move);

	[[nodiscard]] auto poll_move() -> std::optional<Move>;

	[[nodiscard]] auto state() { return m_state.load(); }

  private:
	void receive_loop();

	bnet::Connection m_connection;
	std::thread m_recv_thread{};
	std::atomic<bool> m_running{true};
	std::atomic<ConnectionState> m_state{ConnectionState::Connected};

	std::mutex m_queue_mutex{};
	std::queue<Move> m_incoming{};
};
} // namespace CastleMate