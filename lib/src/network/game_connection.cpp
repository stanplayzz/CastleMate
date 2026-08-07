#include <utility>

#include "castlemate/network/game_connection.hpp"

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
constexpr auto poll_interval_v = std::chrono::milliseconds{100};
}

GameConnection::GameConnection(bnet::Connection connection) : m_connection(std::move(connection)) {
	auto res = m_connection.set_timeout(poll_interval_v); // NOLINT
	if (!res) { return; }

	m_recv_thread = std::jthread{[this](std::stop_token const& token) {
		receive_loop(token);
	}};
}

void GameConnection::send_move(Move move) {
	auto bytes = std::as_bytes(std::span{&move, 1});
	if (auto result = m_connection.send_framed(bytes); !result) { m_state = ConnectionState::Disconnected; }
}

auto GameConnection::poll_move() -> std::optional<Move> {
	std::lock_guard lock{m_queue_mutex};
	if (m_incoming.empty()) { return std::nullopt; }

	auto move = m_incoming.front();
	m_incoming.pop();
	return move;
}

void GameConnection::receive_loop(std::stop_token const& token) {
	auto last_activity = std::chrono::steady_clock::now();

	auto buffer = std::array<std::byte, sizeof(Move)>{};
	while (!token.stop_requested()) {
		auto result = m_connection.receive_framed(buffer);
		if (!result) {
			if (result.error() == bnet::Error::TimedOut) {
				if (std::chrono::steady_clock::now() - last_activity >= 10s) { m_state = ConnectionState::TimedOut; }
				continue;
			}
			m_state = ConnectionState::Disconnected;
			break;
		}
		if (*result != sizeof(Move)) {
			m_state = ConnectionState::Disconnected;
			break;
		}

		last_activity = std::chrono::steady_clock::now();
		auto move = std::bit_cast<Move>(buffer);

		std::lock_guard lock{m_queue_mutex};
		m_incoming.push(move);
	}
}
} // namespace CastleMate