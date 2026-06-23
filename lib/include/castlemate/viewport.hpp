#pragma once
#include <le2d/viewport.hpp>

namespace CastleMate {
constexpr auto viewport_v = le::viewport::Letterbox{.world_size = glm::vec2{512.f}};
} // namespace CastleMate