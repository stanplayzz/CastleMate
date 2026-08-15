#include "castlemate/network/lan_discovery.hpp"
#include "castlemate/utils/constants.hpp"
#include <algorithm>
#include <array>

using namespace std::chrono_literals;

namespace CastleMate {
namespace {
auto const broadcast_addr_v = bnet::Address{.host = "255.255.255.255", .port = lan_port_v};
constexpr std::string_view ping_message_v{"castlemate-client"};
constexpr std::string_view ack_message_v{"castlemate-server"};
} // namespace

void LanDiscovery::start_browsing() {
	reset();
	m_browsing = true;

	auto socket = bnet::UDPSocket::bind(0);
	if (!socket) { throw std::runtime_error{"Failed to bind discovery socket"}; }
	m_discovery_socket = std::move(*socket);
	if (auto result = m_discovery_socket->set_broadcast(true); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
	if (auto result = m_discovery_socket->set_timeout(1ms); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
}

void LanDiscovery::start_advertising() {
	reset();

	auto socket = bnet::UDPSocket::bind(lan_port_v);
	if (!socket) { throw std::runtime_error{"Failed to bind discovery socket"}; }
	m_discovery_socket = std::move(*socket);
	if (auto result = m_discovery_socket->set_timeout(1ms); !result) {
		throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
	}
}

void LanDiscovery::reset() {
	m_discovery_socket.reset();
	m_hosts.clear();
	m_browsing = false;
}

void LanDiscovery::poll() {
	if (!m_discovery_socket) { return; }

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
		} else if (message == ping_message_v) {
			if (auto result = m_discovery_socket->send_to(std::as_bytes(std::span{ack_message_v}), sender); !result) {
				throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
			}
		}
	}

	if (m_browsing && std::chrono::steady_clock::now() - m_last_broadcast > 500ms) {
		m_last_broadcast = std::chrono::steady_clock::now();
		auto const data = std::as_bytes(std::span{ping_message_v});
		if (auto result = m_discovery_socket->send_to(data, broadcast_addr_v); !result) {
			throw std::runtime_error{std::string{bnet::to_string_view(result.error())}};
		}
	}
}
} // namespace CastleMate