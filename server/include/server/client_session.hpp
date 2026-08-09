#pragma once
#include <bnet/connection.hpp>
#include <thread>

namespace server {
struct ClientSession {
	bnet::Connection connection;
	std::jthread worker;
};
} // namespace server