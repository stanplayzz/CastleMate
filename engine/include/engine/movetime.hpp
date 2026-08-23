#pragma once
#include <chrono>

namespace CastleMate::engine {
inline auto compute_movetime(int remaining_ms, int inc_ms, int moves_to_go_estimate = 30) -> std::chrono::milliseconds {
	constexpr auto safety_margin_ms = 50;
	constexpr auto min_budget_ms = 20;

	auto usable = remaining_ms - safety_margin_ms;
	if (usable < min_budget_ms) { return std::chrono::milliseconds{min_budget_ms}; }

	auto budget = (usable / moves_to_go_estimate) + (inc_ms * 4 / 5);

	auto cap = usable * 4 / 10;
	budget = std::min(budget, cap);

	return std::chrono::milliseconds{std::max(budget, min_budget_ms)};
}
} // namespace CastleMate::engine