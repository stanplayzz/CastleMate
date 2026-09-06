#pragma once
#include "engine/transposition.hpp"
#include <chrono>

namespace CastleMate::engine {
struct SearchContext {
	std::chrono::steady_clock::time_point deadline;
	bool timed_out = false;
	int ply = 0;
	TranspositionTable tt{};
	std::array<std::array<Move, 2>, 64> killers{};

	[[nodiscard]] auto check_time() -> bool {
		if (timed_out) { return true; }
		if (std::chrono::steady_clock::now() >= deadline) { timed_out = true; }
		return timed_out;
	}

	void reset(std::chrono::steady_clock::time_point time) {
		deadline = time;
		timed_out = false;
		ply = 0;
		killers = {};
	}
};
} // namespace CastleMate::engine