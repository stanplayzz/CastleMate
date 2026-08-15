#include "server/server.hpp"
#include <shared/message_type.hpp>
#include <algorithm>
#include <print>
#include <thread>

using namespace std::chrono_literals;

namespace server {
namespace {
constexpr auto port_v = 5000;

auto client_to_game(std::unordered_map<std::uint64_t, Game>& games, std::uint64_t id) -> Game* {
	for (auto& [game_id, game] : games) {
		if (game.white_id == id || game.black_id == id) { return &game; }
	}
	return nullptr;
}

auto join_queue(std::vector<std::uint64_t>& queue, std::uint64_t id) {
	if (std::ranges::find(queue, id) != queue.end()) { return; }
	queue.push_back(id);
	std::println("[INFO] User with ID: {} added to matchmaking queue", id);
}

auto leave_queue(std::vector<std::uint64_t>& queue, std::uint64_t id) { std::erase(queue, id); }
} // namespace

Server::Server() {
	auto listener = bnet::Listener::create(port_v);
	if (!listener) {
		throw std::runtime_error{std::format("Failed to create listener, {}", bnet::to_string_view(listener.error()))};
	}
	m_listener = std::make_unique<bnet::Listener>(std::move(*listener));
	if (auto res = m_listener->set_blocking(false); !res) {
		throw std::runtime_error{std::string{bnet::to_string_view(res.error())}};
	}
}

void Server::run() {
	m_matchmaking_thread = std::jthread{[this](std::stop_token const& token) {
		while (!token.stop_requested()) {
			matchmaking_tick();
			std::this_thread::sleep_for(200ms);
		}
	}};

	for (;;) {
		cleanup_clients();

		auto connection = m_listener->accept();
		if (!connection) {
			std::this_thread::sleep_for(5ms);
			continue;
		}

		if (auto res = connection->set_timeout(500ms); !res) { continue; }

		auto const id = m_next_id.fetch_add(1, std::memory_order_relaxed);

		auto client = std::make_shared<ClientSession>();
		client->connection = std::make_shared<bnet::Connection>(std::move(*connection));

		{
			std::lock_guard lock{m_mutex};
			m_clients.emplace(id, client);
		}

		client->worker = std::jthread{[this, id](std::stop_token const& token) {
			handle_client(token, id);
		}};
	}
}

void Server::matchmaking_tick() {
	std::lock_guard lock{m_mutex};
	while (m_queue.size() >= 2) {
		auto const id_a = m_queue.at(0);
		auto const id_b = m_queue.at(1);
		m_queue.erase(m_queue.begin(), m_queue.begin() + 2);

		auto it_a = m_clients.find(id_a);
		auto it_b = m_clients.find(id_b);
		if (it_a == m_clients.end() || it_b == m_clients.end()) {
			if (it_a != m_clients.end()) { m_queue.push_back(id_a); }
			if (it_b != m_clients.end()) { m_queue.push_back(id_b); }
			continue;
		}

		auto const game_id = m_next_game_id.fetch_add(1, std::memory_order_relaxed);

		auto msg_a_v = shared::MatchFoundMsg{.game_id = game_id, .white = true};
		auto msg_b_v = shared::MatchFoundMsg{.game_id = game_id, .white = false};

		(void)it_a->second->connection->send_framed(shared::match_found_to_bytes(msg_a_v));
		(void)it_b->second->connection->send_framed(shared::match_found_to_bytes(msg_b_v));

		m_games.emplace(game_id, Game{.game_id = game_id, .white_id = id_a, .black_id = id_b});

		std::println("[INFO] Matched {} (white) vs {} (black) into game {}", id_a, id_b, game_id);
	}
}

void Server::handle_client(std::stop_token const& token, std::uint64_t id) {
	std::shared_ptr<bnet::Connection> connection{};
	{
		std::lock_guard lock{m_mutex};
		auto it = m_clients.find(id);
		if (it == m_clients.end()) { return; }

		connection = it->second->connection;
	}
	auto buffer = std::array<std::byte, 4096>{};

	while (!token.stop_requested()) {
		auto result = connection->receive_framed(buffer);
		if (!result) {
			if (result.error() == bnet::Error::TimedOut) { continue; }
			break;
		}
		on_client_message(id, std::span{buffer.data(), *result});
	}

	std::lock_guard lock{m_mutex};
	auto it = m_clients.find(id);
	if (it == m_clients.end()) { return; }

	it->second->connection = nullptr;
}

void Server::cleanup_clients() {
	std::lock_guard lock{m_mutex};
	for (auto it = m_clients.begin(); it != m_clients.end();) {
		if (!it->second->connection) {
			std::println("[INFO] Removing player with id: {}", it->first);
			std::erase(m_queue, it->first);
			it = m_clients.erase(it);

		} else {
			++it;
		}
	}
}

void Server::on_client_message(std::uint64_t id, std::span<std::byte const> data) {
	if (data.empty()) { return; }

	auto const type = static_cast<shared::MsgType>(data[0]);
	auto const payload = data.subspan(1);

	std::lock_guard lock{m_mutex};

	switch (type) {
	case shared::MsgType::JoinQueue: join_queue(m_queue, id); break;
	case shared::MsgType::LeaveQueue: leave_queue(m_queue, id); break;
	case shared::MsgType::Move: handle_move(id, payload); break;
	case shared::MsgType::DrawOffer: handle_draw_offer(id); break;
	case shared::MsgType::DrawAccept: handle_draw_accepted(id); break;
	case shared::MsgType::Resign: handle_resign(id); break;
	default: break;
	}
}

void Server::handle_move(std::uint64_t id, std::span<std::byte const> data) {
	auto* game = client_to_game(m_games, id);
	if (!game) { return; }

	auto const other_id = (id == game->white_id) ? game->black_id : game->white_id;

	auto it = m_clients.find(other_id);
	if (it == m_clients.end()) { return; }

	auto buffer = std::vector<std::byte>(data.size() + 1);
	buffer[0] = std::byte{std::to_underlying(shared::MsgType::Move)};
	std::ranges::copy(data, buffer.begin() + 1);

	if (auto res = it->second->connection->send_framed(buffer); !res) {
		std::println("[ERROR] Failed to send message: {}", bnet::to_string_view(res.error()));
	}
}

void Server::handle_draw_offer(std::uint64_t id) {
	auto* game = client_to_game(m_games, id);
	if (!game) { return; }

	auto const other_id = (id == game->white_id) ? game->black_id : game->white_id;
	auto it = m_clients.find(other_id);
	if (it == m_clients.end()) { return; }

	auto const msg = std::array{std::byte{std::to_underlying(shared::MsgType::DrawOffer)}};
	if (auto res = it->second->connection->send_framed(msg); !res) {
		std::println("[ERROR] Failed to send message: {}", bnet::to_string_view(res.error()));
	}
}

void Server::handle_draw_accepted(std::uint64_t id) {
	auto* game = client_to_game(m_games, id);
	if (!game) { return; }

	auto const other_id = (id == game->white_id) ? game->black_id : game->white_id;
	auto it = m_clients.find(other_id);
	if (it == m_clients.end()) { return; }

	auto const msg = shared::GameOverMsg{.reason = shared::GameOverReason::Draw};

	if (auto res = it->second->connection->send_framed(shared::game_over_to_bytes(msg)); !res) {
		std::println("[ERROR] Failed to send message: {}", bnet::to_string_view(res.error()));
	}

	std::println("[INFO] Removing game with id: {}", game->game_id);
	m_games.erase(game->game_id);
}

void Server::handle_resign(std::uint64_t id) {
	auto* game = client_to_game(m_games, id);
	if (!game) { return; }

	auto const other_id = (id == game->white_id) ? game->black_id : game->white_id;
	auto it = m_clients.find(other_id);
	if (it == m_clients.end()) { return; }

	auto const msg = shared::GameOverMsg{
		.reason = shared::GameOverReason::Resign,
		.white_won = (id != game->white_id),
	};

	if (auto res = it->second->connection->send_framed(shared::game_over_to_bytes(msg)); !res) {
		std::println("[ERROR] Failed to send message: {}", bnet::to_string_view(res.error()));
	}

	std::println("[INFO] Removing game with id: {}", game->game_id);
	m_games.erase(game->game_id);
}
} // namespace server