#pragma once
#include "castlemate/core/color.hpp"
#include "castlemate/core/piece.hpp"
#include <deque>
#include <string>
#include <string_view>

namespace CastleMate {
struct StateInfo {
	StateInfo* previous{};

	Piece moved{COUNT_};
	Piece captured{COUNT_};
	std::uint8_t captured_sq{};

	int prev_en_passant{-1};

	bool prev_castle_wk{};
	bool prev_castle_wq{};
	bool prev_castle_bk{};
	bool prev_castle_bq{};

	int prev_halfmove_clock{};

	std::uint64_t prev_hash{};
	std::uint64_t hash{};
	bool irreversible{};
};

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

	int halfmove_clock{};

	std::uint64_t hash{};

	StateInfo* state{};
	std::deque<StateInfo> states{};

	static auto from_fen(std::string_view fen) -> Position;
	[[nodiscard]] auto to_fen() const -> std::string;
};
} // namespace CastleMate