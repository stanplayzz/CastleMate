#include "castlemate/network/lan_session.hpp"
#include <algorithm>
#include <array>
#include <print>

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
constexpr std::uint16_t discovery_port_v{5000};
constexpr std::uint16_t game_port_v{5001};
constexpr std::string_view ping_message_v{"castlemate-join-lan"};
constexpr std::string_view ack_message_v{"castlemate-server-here"};
} // namespace

void LanSession::reset() {
	m_role = Role::None;
	m_discovery_socket.reset();
	m_listener.reset();
	m_hosts.clear();
	m_target_host.reset();
}

void LanSession::start_hosting() {
	reset();
	m_role = Role::Hosting;

	auto socket = bnet::UDPSocket::bind(discovery_port_v);
	if (!socket) { throw std::runtime_error{"Failed to bind discovery socket"}; }
	m_discovery_socket = std::move(*socket);
	if (auto result = m_discovery_socket->set_timeout(1ms); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}

	auto listener = bnet::Listener::create(game_port_v, 4);
	if (!listener) { throw std::runtime_error{std::string{bnet::to_string_view(listener.error())}}; }
	m_listener = std::move(*listener);
	if (auto result = m_listener->set_blocking(false); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
}

void LanSession::start_joining() {
	reset();
	m_role = Role::Joining;

	auto socket = bnet::UDPSocket::bind(0);
	if (!socket) { throw std::runtime_error{"Failed to bind discovery socket"}; }
	m_discovery_socket = std::move(*socket);
	if (auto result = m_discovery_socket->set_broadcast(true); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
	if (auto result = m_discovery_socket->set_timeout(1ms); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}

	auto const data = std::as_bytes(std::span{ping_message_v});
	auto const broadcast_addr = bnet::Address{.host = "255.255.255.255", .port = discovery_port_v};
	if (auto result = m_discovery_socket->send_to(data, broadcast_addr); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
}

void LanSession::connect_to(bnet::Address const& host) {
	std::println("connect_to called: {}:{}", host.host, std::to_string(host.port));
	m_target_host = host;
}

auto LanSession::poll() -> std::optional<bnet::Connection> {
	switch (m_role) {
	case Role::Hosting: return poll_host();
	case Role::Joining: return poll_join();
	case Role::None: return std::nullopt;
	}
	return std::nullopt;
}

auto LanSession::poll_host() -> std::optional<bnet::Connection> {
	if (!m_discovery_socket || !m_listener) { return std::nullopt; }

	auto buffer = std::array<std::byte, 64>{};
	auto sender = bnet::Address{};
	if (auto n = m_discovery_socket->receive_from(buffer, sender); n) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto const message = std::string_view{reinterpret_cast<char const*>(buffer.data()), *n};
		if (message == ping_message_v) {
			if (auto result = m_discovery_socket->send_to(std::as_bytes(std::span{ack_message_v}), sender); !result) {
				throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
			}
		}
	}

	if (auto conn = m_listener->accept(); conn) {
		reset();
		return std::move(*conn);
	}
	return std::nullopt;
}

auto LanSession::poll_join() -> std::optional<bnet::Connection> {
	if (m_target_host) { return poll_connecting(); }
	return poll_browsing();
}

auto LanSession::poll_browsing() -> std::optional<bnet::Connection> {
	if (!m_discovery_socket) { return std::nullopt; }

	auto buffer = std::array<std::byte, 64>{};
	auto sender = bnet::Address{};
	if (auto n = m_discovery_socket->receive_from(buffer, sender); n) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto const message = std::string_view{reinterpret_cast<char const*>(buffer.data()), *n};
		if (message == ack_message_v) {
			auto const already_known = std::ranges::any_of(m_hosts, [&](bnet::Address const& h) {
				return h.host == sender.host;
			});
			if (!already_known) { m_hosts.push_back(sender); }
		}
	}
	return std::nullopt;
}

auto LanSession::poll_connecting() -> std::optional<bnet::Connection> {
	auto connect_addr = bnet::Address{.host = m_target_host->host, .port = game_port_v};
	if (auto conn = bnet::Connection::connect(connect_addr); conn) {
		std::println("connect succeeded");
		reset();
		return std::move(*conn);
	}
	std::println("connect failed, resetting target");
	m_target_host.reset();
	return std::nullopt;
}

} // namespace CastleMate