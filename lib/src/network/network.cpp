#include "castlemate/network/network.hpp"
#include "castlemate/utils/constants.hpp"
#include <shared/message_type.hpp>

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
constexpr auto msg_to_bytes(shared::MsgType msg) { return std::as_bytes(std::span{&msg, 1}); }
} // namespace

void Network::host_lan() {
	reset();

	auto listener = bnet::Listener::create(lan_port_v, 1);
	if (!listener) { throw std::runtime_error{"Failed to create listener"}; }
	m_listener = std::make_unique<bnet::Listener>(std::move(*listener));
	if (auto res = m_listener->set_blocking(false); !res) {
		throw std::runtime_error{std::string{bnet::to_string_view(res.error())}};
	}

	m_lan_discovery.start_advertising();
}

void Network::browse_lan() {
	reset();
	m_lan_discovery.start_browsing();
}

void Network::join_lan(bnet::Address const& host) {
	reset();

	auto connection = bnet::Connection::connect({.host = host.host, .port = lan_port_v});
	if (!connection) { throw std::runtime_error{"Failed to create connection"}; }
	m_connection = std::make_unique<bnet::Connection>(std::move(*connection));
	m_matched_game = shared::MatchFoundMsg{.game_id = 0, .white = false};
}

void Network::search_match() {
	reset();

	auto connection = bnet::Connection::connect({.host = server_host, .port = game_port_v});
	if (!connection) { throw std::runtime_error{std::string{bnet::to_string_view(connection.error())}}; }
	m_connection = std::make_unique<bnet::Connection>(std::move(*connection));

	(void)m_connection->set_timeout(1ms);

	(void)m_connection->send_framed(msg_to_bytes(shared::MsgType::JoinQueue));
}

void Network::cancel_search() { (void)m_connection->send_framed(msg_to_bytes(shared::MsgType::LeaveQueue)); }

void Network::update() {
	m_lan_discovery.poll();

	if (m_listener) {
		if (auto connection = m_listener->accept(); connection) {
			m_matched_game = shared::MatchFoundMsg{.game_id = 0, .white = true};
			m_connection = std::make_unique<bnet::Connection>(std::move(*connection));
		}
	}

	if (m_connection && !m_matched_game) {
		auto buffer = std::array<std::byte, 64>{};
		if (auto result = m_connection->receive_framed(buffer); result) {
			auto const type = static_cast<shared::MsgType>(buffer[0]);
			if (type == shared::MsgType::MatchFound) {
				// NOLINTNEXTLINE
				m_matched_game = shared::bytes_to_match_found(std::span{buffer.data(), *result});
				if (m_matched_game->game_id == 0) { m_matched_game = std::nullopt; }
			}
		}
	}
}

void Network::reset() {
	m_connection.reset();
	m_listener.reset();
	m_lan_discovery.reset();
	m_matched_game.reset();
}
} // namespace CastleMate