#pragma once
#include "castlemate/utils/constants.hpp"
#include <glm/common.hpp>

namespace CastleMate {
constexpr auto screen_to_sq(glm::vec2 screen_pos) -> glm::vec2 {
	auto floor = glm::floor((screen_pos / viewport_v.world_size) * glm::vec2{8, 8});
	return {floor.x, 7 - floor.y}; // flip y axis for A1 = 0,0
}

constexpr auto window_to_world(glm::vec2 window, glm::vec2 window_size) {
	auto const viewport_size = viewport_v.fill_target_space(window_size);
	auto const offset = (window_size - viewport_size) * 0.5f;
	return (window - offset) / viewport_size * viewport_v.world_size * glm::vec2{1, -1} -
		   viewport_v.world_size * glm::vec2{0.5f, -0.5f};
}

constexpr auto window_to_board(glm::vec2 window, glm::vec2 window_size) {
	auto const viewport_size = viewport_v.fill_target_space(window_size);
	auto const offset = (window_size - viewport_size) * 0.5f;
	return (window - offset) / viewport_size * viewport_v.world_size;
}
} // namespace CastleMate