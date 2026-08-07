#include <bnet/listener.hpp>
#include <print>

namespace {
constexpr auto server_port_v = 5000;
}

auto main() -> int {
	auto listener = bnet::Listener::create(server_port_v);
	if (!listener) { throw std::runtime_error{std::string{bnet::to_string_view(listener.error())}}; }

	for (;;) {
		auto connection = listener->accept();
		if (!connection) { continue; }

		std::println("Connection accepted.");
	}

	return EXIT_SUCCESS;
}