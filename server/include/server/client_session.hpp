#pragma once
#include <bnet/connection.hpp>
#include <thread>

namespace server {
struct ClientSession {
	std::shared_ptr<bnet::Connection> connection;
	std::jthread worker;
};
} // namespace server