#pragma once
#include "castlemate/network/lan_discovery.hpp"
#include <bnet/context.hpp>

namespace CastleMate {
class Network {
  public:
	Network() = default;

	// LAN
	void host_lan();
	void browse_lan();
	void join_lan(bnet::Address const& host);
	void stop_browse_lan() { m_lan_discovery.reset(); }

	// Online matchmaking
	void search_match();
	void cancel_search();

	void update();

	void reset();

	[[nodiscard]] auto get_connection() -> bnet::Connection* { return m_connection.get(); }
	[[nodiscard]] auto lan_hosts() const -> std::span<bnet::Address const> { return m_lan_discovery.hosts(); }

  private:
	void update_matchmaking();

	LanDiscovery m_lan_discovery{};

	std::unique_ptr<bnet::Listener> m_listener{};
	std::unique_ptr<bnet::Connection> m_connection{};

	bnet::Context m_context{};
};
} // namespace CastleMate