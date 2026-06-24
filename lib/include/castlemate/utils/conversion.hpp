#pragma once
#include "castlemate/utils/constants.hpp"
#include <glm/common.hpp>

namespace CastleMate {
constexpr auto screen_to_sq(glm::vec2 screen_pos) -> glm::vec2 {
	auto floor = glm::floor((screen_pos / viewport_v.world_size) * glm::vec2{8, 8});
	return {floor.x, 7 - floor.y}; // flip y axis for A1 = 0,0
}
} // namespace CastleMate