#pragma once
#include "server/client_session.hpp"
#include <bnet/listener.hpp>
#include <mutex>
#include <unordered_map>

namespace server {
class Server {
  public:
	Server();
	void run();

  private:
	void handle_client(std::stop_token const& token, std::uint64_t id);
	void remove_client(std::uint64_t id);
	void on_client_message(std::uint64_t id, std::span<std::byte const> data);

	std::unique_ptr<bnet::Listener> m_listener{};
	std::mutex m_mutex{};

	std::atomic<std::uint64_t> m_next_id{1};
	std::unordered_map<std::uint64_t, ClientSession> m_clients{};
	std::vector<std::uint64_t> m_queue{};
};
} // namespace server