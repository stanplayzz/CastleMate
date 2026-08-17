#pragma once
#include "castlemate/core/color.hpp"
#include "castlemate/core/piece.hpp"
#include <string_view>

namespace CastleMate {
struct Position {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
	std::uint64_t bb[COUNT_]{};

	std::uint64_t white_occ{};
	std::uint64_t black_occ{};
	std::uint64_t occ{};

	Color turn{Color::White};

	int en_passant{-1};
	bool castle_wk{true};
	bool castle_wq{true};
	bool castle_bk{true};
	bool castle_bq{true};

	static auto from_fen(std::string_view fen) -> Position;
	[[nodiscard]] auto to_fen() const -> std::string;
};
} // namespace CastleMate