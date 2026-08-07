#include "castlemate/network/network.hpp"
#include "castlemate/utils/constants.hpp"

namespace CastleMate {
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
}

void Network::search_match() {
	reset();
	auto connection = bnet::Connection::connect({.host = server_host, .port = server_port_v});
}

void Network::update() {
	m_lan_discovery.poll();

	if (m_listener) {
		if (auto connection = m_listener->accept(); connection) {
			m_connection = std::make_unique<bnet::Connection>(std::move(*connection));
		}
	}
}

void Network::reset() {
	m_connection.reset();
	m_listener.reset();
	m_lan_discovery.reset();
}
} // namespace CastleMate