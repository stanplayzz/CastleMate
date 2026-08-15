#pragma once
#include <bnet/connection.hpp>
#include <bnet/listener.hpp>
#include <bnet/socket.hpp>
#include <optional>

namespace CastleMate {
class LanDiscovery {
  public:
	void start_browsing();
	void start_advertising();

	void reset();

	void poll();

	[[nodiscard]] auto active() const -> bool { return m_discovery_socket.has_value(); }

	[[nodiscard]] auto hosts() const -> std::span<bnet::Address const> { return m_hosts; }

  private:
	std::optional<bnet::UDPSocket> m_discovery_socket{};
	std::vector<bnet::Address> m_hosts{};
	std::chrono::steady_clock::time_point m_last_broadcast{};
	bool m_browsing{};
};

} // namespace CastleMate