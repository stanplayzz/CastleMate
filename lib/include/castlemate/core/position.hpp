#pragma once
#include "castlemate/core/piece.hpp"

namespace CastleMate {
struct Position {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
	std::uint64_t bb[COUNT_]{};

	std::uint64_t white_occ{};
	std::uint64_t black_occ{};
	std::uint64_t occ{};
};
} // namespace CastleMate