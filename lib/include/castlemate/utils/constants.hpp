#pragma once

#include "le2d/viewport.hpp"
namespace CastleMate {
constexpr auto viewport_v = le::viewport::Letterbox{.world_size = glm::vec2{512.f}};
constexpr auto tile_size_v = viewport_v.world_size / 8.f;
} // namespace CastleMate