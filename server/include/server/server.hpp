#pragma once
#include "server/client_session.hpp"
#include <bnet/listener.hpp>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace server {
struct Game {
	std::uint64_t game_id{};
	std::uint64_t white_id{};
	std::uint64_t black_id{};
};

class Server {
  public:
	Server();
	void run();

  private:
	void matchmaking_tick();

	void handle_client(std::stop_token const& token, std::uint64_t id);
	void cleanup_clients();
	void on_client_message(std::uint64_t id, std::span<std::byte const> data);

	void handle_move(std::uint64_t id, std::span<std::byte const> data);
	void handle_draw_offer(std::uint64_t id);
	void handle_draw_accepted(std::uint64_t id);
	void handle_resign(std::uint64_t id);

	std::unique_ptr<bnet::Listener> m_listener{};
	std::mutex m_mutex{};

	std::atomic<std::uint64_t> m_next_game_id{1};
	std::unordered_map<std::uint64_t, Game> m_games{};

	std::atomic<std::uint64_t> m_next_id{1};
	std::unordered_map<std::uint64_t, std::shared_ptr<ClientSession>> m_clients{};
	std::vector<std::uint64_t> m_queue{};

	std::jthread m_matchmaking_thread{};
};
} // namespace server