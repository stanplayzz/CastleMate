#include "castlemate/network/game_connection.hpp"
#include "shared/message_type.hpp"

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
constexpr auto poll_interval_v = std::chrono::milliseconds{100};
}

GameConnection::GameConnection(gsl::not_null<bnet::Connection*> connection) : m_connection(connection) {
	auto res = m_connection->set_timeout(poll_interval_v); // NOLINT
	if (!res) { return; }

	m_recv_thread = std::jthread{[this](std::stop_token const& token) {
		receive_loop(token);
	}};
}

void GameConnection::send_move(Move move) {
	auto buffer = std::array<std::byte, 1 + sizeof(move)>{};
	buffer[0] = std::byte{std::to_underlying(shared::MsgType::Move)};
	std::memcpy(buffer.data() + 1, &move, sizeof(move)); // NOLINT
	if (auto result = m_connection->send_framed(buffer); !result) { m_state = ConnectionState::Disconnected; }
}

void GameConnection::resign() {
	auto buffer = std::array<std::byte, 1>{std::byte{std::to_underlying(shared::MsgType::Resign)}};
	if (auto result = m_connection->send_framed(buffer); !result) { m_state = ConnectionState::Disconnected; }
}

void GameConnection::offer_draw() {
	auto buffer = std::array<std::byte, 1>{std::byte{std::to_underlying(shared::MsgType::DrawOffer)}};
	if (auto result = m_connection->send_framed(buffer); !result) { m_state = ConnectionState::Disconnected; }
}

void GameConnection::accept_draw() {
	auto buffer = std::array<std::byte, 1>{std::byte{std::to_underlying(shared::MsgType::DrawAccept)}};
	if (auto result = m_connection->send_framed(buffer); !result) { m_state = ConnectionState::Disconnected; }
}

auto GameConnection::poll_event() -> std::optional<IncomingEvent> {
	std::lock_guard lock{m_queue_mutex};
	if (m_incoming.empty()) { return std::nullopt; }

	auto event = m_incoming.front();
	m_incoming.pop();
	return event;
}

void GameConnection::receive_loop(std::stop_token const& token) {
	auto last_activity = std::chrono::steady_clock::now();

	auto buffer = std::array<std::byte, 64>{};
	while (!token.stop_requested()) {
		auto result = m_connection->receive_framed(buffer);
		if (!result) {
			if (result.error() == bnet::Error::TimedOut) {
				if (std::chrono::steady_clock::now() - last_activity >= 10s) { m_state = ConnectionState::TimedOut; }
				continue;
			}
			m_state = ConnectionState::Disconnected;
			break;
		}

		if (*result < 1) { continue; }

		auto const type = static_cast<shared::MsgType>(buffer[0]);
		auto const received = std::span{buffer.data(), *result};

		std::optional<IncomingEvent> event;

		switch (type) {
		case shared::MsgType::Move: {
			if (*result < 1 + sizeof(Move)) { continue; }
			event = IncomingEvent{
				.kind = IncomingEvent::Kind::Move,
				.move =
					Move{
						.from = std::to_integer<std::uint8_t>(received[1]),
						.to = std::to_integer<std::uint8_t>(received[2]),
						.promotion = static_cast<Piece>(received[3]),
					},
			};
			break;
		}
		case shared::MsgType::DrawOffer: {
			event = IncomingEvent{.kind = IncomingEvent::Kind::DrawOffer};
			break;
		}
		case shared::MsgType::GameOver: {
			event =
				IncomingEvent{.kind = IncomingEvent::Kind::GameOver, .game_over = shared::bytes_to_game_over(received)};
			break;
		}
		default: continue;
		}

		last_activity = std::chrono::steady_clock::now();

		std::lock_guard lock{m_queue_mutex};
		m_incoming.push(*event);
	}
}
} // namespace CastleMate