#pragma once
#include <chrono>

namespace CastleMate::engine {
struct SearchParams {
	std::optional<std::chrono::milliseconds> movetime{};
	std::optional<int> depth{};
};
} // namespace CastleMate::engine