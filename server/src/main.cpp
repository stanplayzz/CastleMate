#include "server/server.hpp"
#include <print>

auto main() -> int {
	try {
		server::Server{}.run();
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}