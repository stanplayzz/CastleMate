#pragma once
#include <bnet/connection.hpp>
#include <bnet/listener.hpp>
#include <bnet/socket.hpp>
#include <optional>

namespace CastleMate {
class LanSession {
  public:
	enum class Role : std::int8_t { None, Hosting, Joining };

	void start_hosting();
	void start_joining();

	[[nodiscard]] auto poll() -> std::optional<bnet::Connection>;
	[[nodiscard]] auto active() const -> bool { return m_role != Role::None; }
	[[nodiscard]] auto is_host() const -> bool { return m_role == Role::Hosting; }

	[[nodiscard]] auto discovered_hosts() const -> std::span<bnet::Address const> { return m_hosts; }

	void connect_to(bnet::Address const& host);

	void reset();

  private:
	[[nodiscard]] auto poll_host() -> std::optional<bnet::Connection>;
	[[nodiscard]] auto poll_join() -> std::optional<bnet::Connection>;
	[[nodiscard]] auto poll_browsing() -> std::optional<bnet::Connection>;
	[[nodiscard]] auto poll_connecting() -> std::optional<bnet::Connection>;

	Role m_role{Role::None};
	std::optional<bnet::UDPSocket> m_discovery_socket{};
	std::optional<bnet::Listener> m_listener{};
	std::vector<bnet::Address> m_hosts{};
	std::optional<bnet::Address> m_target_host{};
};

} // namespace CastleMate