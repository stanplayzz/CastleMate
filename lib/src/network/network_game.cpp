#include "castlemate/network/network_game.hpp"
#include <print>

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
constexpr auto poll_interval_v = std::chrono::milliseconds{100};
}

NetworkGame::NetworkGame(bnet::Connection connection) : m_connection(std::move(connection)) {
	auto res = m_connection.set_timeout(poll_interval_v); // NOLINT
	if (!res) {
		std::println("TEST {}", bnet::to_string_view(res.error()));
		return;
	}
	m_recv_thread = std::thread{[this] {
		receive_loop();
	}};
}

NetworkGame::~NetworkGame() {
	m_running = false;
	if (m_recv_thread.joinable()) { m_recv_thread.join(); }
}

void NetworkGame::send_move(Move move) {
	auto bytes = std::as_bytes(std::span{&move, 1});
	if (auto result = m_connection.send_framed(bytes); !result) { m_state = ConnectionState::Disconnected; }
}

auto NetworkGame::poll_move() -> std::optional<Move> {
	std::lock_guard lock{m_queue_mutex};
	if (m_incoming.empty()) { return std::nullopt; }

	auto move = m_incoming.front();
	m_incoming.pop();
	return move;
}

void NetworkGame::receive_loop() {
	auto last_activity = std::chrono::steady_clock::now();

	auto buffer = std::array<std::byte, sizeof(Move)>{};
	while (m_running) {
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