#pragma once
#include <chrono>

namespace CastleMate::engine {
struct SearchContext {
	std::chrono::steady_clock::time_point deadline;
	bool timed_out = false;

	[[nodiscard]] auto check_time() -> bool {
		if (timed_out) { return true; }
		if (std::chrono::steady_clock::now() >= deadline) { timed_out = true; }
		return timed_out;
	}
};
} // namespace CastleMate::engine