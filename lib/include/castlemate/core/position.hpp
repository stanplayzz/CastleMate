#pragma once
#include "castlemate/core/piece.hpp"

namespace CastleMate {
struct Position {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
	std::uint64_t bb[COUNT_]{};

	std::uint64_t white_occ{};
	std::uint64_t black_occ{};
	std::uint64_t occ{};

	int en_passant{};
	bool castle_wk{true};
	bool castle_wq{true};
	bool castle_bk{true};
	bool castle_bq{true};
};
} // namespace CastleMate