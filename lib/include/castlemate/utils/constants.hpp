#pragma once

#include "le2d/viewport.hpp"
namespace CastleMate {
constexpr auto viewport_v = le::viewport::Letterbox{.world_size = glm::vec2{2560.f, 2048.f}};
constexpr auto board_size_v = glm::vec2{2048, 2048};
constexpr auto tile_size_v = board_size_v / 8.f;

// network
constexpr auto game_port_v = 5000;
constexpr auto lan_port_v = 5001;
constexpr auto server_port_v = 5002;
constexpr auto server_host = "castlemate.duckdns.org";
} // namespace CastleMate