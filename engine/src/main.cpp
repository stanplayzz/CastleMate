#include "engine/engine.hpp"
#include <exception>
#include <print>

auto main() -> int {
	try {
		CastleMate::engine::Engine{}.run();
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}