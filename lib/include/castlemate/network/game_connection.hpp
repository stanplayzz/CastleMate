#pragma once
#include "castlemate/core/move.hpp"
#include <bnet/connection.hpp>
#include <shared/message_type.hpp>
#include <gsl/pointers>
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
	struct IncomingEvent {
		enum class Kind : std::uint8_t { Move, DrawOffer, GameOver } kind{};
		Move move{};
		shared::GameOverMsg game_over{};
	};

	explicit GameConnection(gsl::not_null<bnet::Connection*> connection);

	void send_move(Move move);

	void resign();

	void offer_draw();

	void accept_draw();

	[[nodiscard]] auto poll_event() -> std::optional<IncomingEvent>;

	[[nodiscard]] auto state() { return m_state.load(); }

  private:
	void receive_loop(std::stop_token const& token);

	bnet::Connection* m_connection;
	std::atomic<ConnectionState> m_state{ConnectionState::Connected};

	std::mutex m_queue_mutex{};
	std::queue<IncomingEvent> m_incoming{};

	std::jthread m_recv_thread{};
};
} // namespace CastleMate