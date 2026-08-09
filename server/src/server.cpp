#include "server/server.hpp"
#include <shared/message_type.hpp>
#include <algorithm>
#include <print>
#include <thread>

using namespace std::chrono_literals;

namespace server {
namespace {
constexpr auto port_v = 5000;

auto join_queue(std::vector<std::uint64_t>& queue, std::uint64_t id) {
	if (std::ranges::find(queue, id) != queue.end()) { return; }
	queue.push_back(id);
	std::println("User with id {} added to queue", id);
}
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
	for (;;) {
		auto connection = m_listener->accept();
		if (!connection) {
			std::this_thread::sleep_for(5ms);
			continue;
		}

		if (auto res = connection->set_timeout(500ms); !res) { continue; }

		auto const id = m_next_id.fetch_add(1, std::memory_order_relaxed);

		std::lock_guard lock{m_mutex};
		auto [it, _] = m_clients.try_emplace(id, ClientSession{.connection = std::move(*connection), .worker = {}});
		it->second.worker = std::jthread{[this, id](std::stop_token const& token) {
			handle_client(token, id);
		}};
	}
}

void Server::handle_client(std::stop_token const& token, std::uint64_t id) {
	auto& connection = m_clients.at(id).connection;

	auto const address = connection.remote_address();
	auto buffer = std::array<std::byte, 4096>{};

	while (!token.stop_requested()) {
		auto result = connection.receive_framed(buffer);
		if (!result) {
			if (result.error() == bnet::Error::TimedOut) { continue; }
		}
		break;
	}
	remove_client(id);
}

void Server::remove_client(std::uint64_t id) {
	std::lock_guard lock{m_mutex};
	auto it = m_clients.find(id);
	if (it == m_clients.end()) { return; }

	it->second.worker.request_stop();
	it->second.worker.detach();
	std::erase(m_queue, id);
}

void Server::on_client_message(std::uint64_t id, std::span<std::byte const> data) {
	if (data.empty()) { return; }
	auto const type = static_cast<shared::MsgType>(data[0]);
	auto const payload = data.subspan(1);

	switch (type) {
	case shared::MsgType::JoinQueue: join_queue(m_queue, id);
	case shared::MsgType::LeaveQueue: break;
	case shared::MsgType::Move: break;
	case shared::MsgType::DrawOffer: break;
	case shared::MsgType::Resign: break;
	default: break;
	}
}
} // namespace server