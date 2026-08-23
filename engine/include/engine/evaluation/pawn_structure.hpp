#pragma once
#include "castlemate/utils/bit_math.hpp"
#include <array>

namespace CastleMate::engine {
inline constexpr auto file_masks_v = []() {
	auto masks = std::array<std::uint64_t, 8>{};
	for (auto f = 0; f < 8; ++f) {
		auto mask = std::uint64_t{};
		for (auto r = 0; r < 8; ++r) { set_bit(mask, (r * 8) + f); }
		masks.at(static_cast<std::size_t>(f)) = mask;
	}
	return masks;
}();

inline constexpr auto adjacent_file_masks_v = []() {
	auto masks = std::array<std::uint64_t, 8>{};
	for (std::size_t f = 0; f < 8; ++f) {
		auto mask = std::uint64_t{};
		if (f > 0) { mask |= file_masks_v.at(f - 1); }
		if (f < 7) { mask |= file_masks_v.at(f + 1); }
		masks.at(f) = mask;
	}
	return masks;
}();

inline constexpr auto white_passed_mask_v = []() {
	std::array<std::uint64_t, 64> masks{};
	for (std::size_t sq = 0; sq < 64; ++sq) {
		auto file = sq % 8;
		auto rank = sq / 8;

		std::uint64_t mask = 0;
		for (std::size_t r = rank + 1; r < 8; ++r) {
			mask |= file_masks_v.at(file);
			if (file > 0) { mask |= file_masks_v.at(file - 1); }
			if (file < 7) { mask |= file_masks_v.at(file + 1); }
		}
		masks.at(sq) = mask;
	}
	return masks;
}();

inline constexpr auto black_passed_mask_v = []() {
	std::array<std::uint64_t, 64> masks{};
	for (std::size_t sq = 0; sq < 64; ++sq) {
		auto file = sq % 8;
		auto rank = sq / 8;

		std::uint64_t mask = 0;
		for (std::size_t r = 0; r < rank; ++r) {
			mask |= file_masks_v.at(file);
			if (file > 0) { mask |= file_masks_v.at(file - 1); }
			if (file < 7) { mask |= file_masks_v.at(file + 1); }
		}
		masks.at(sq) = mask;
	}
	return masks;
}();

inline auto doubled_pawn_penalty(std::uint64_t pawns) -> int {
	auto penalty = 0;
	for (std::size_t f = 0; f < 8; ++f) {
		auto count = std::popcount(pawns & file_masks_v.at(f));
		if (count > 1) { penalty += (count - 1) * 10; }
	}
	return penalty;
}

inline auto isolated_pawn_penalty(std::uint64_t pawns) -> int {
	auto penalty = 0;
	for (std::size_t f = 0; f < 8; ++f) {
		if ((pawns & file_masks_v.at(f)) && !(pawns & adjacent_file_masks_v.at(f))) {
			penalty += std::popcount(pawns & file_masks_v.at(f)) * 15;
		}
	}
	return penalty;
}

inline auto passed_pawn_bonus(std::uint64_t friendly, std::uint64_t enemy, bool white) {
	auto bonus = 0;
	auto bits = friendly;
	while (bits) {
		auto sq = pop_lsb(bits);
		auto const& mask = white ? white_passed_mask_v.at(sq) : black_passed_mask_v.at(sq);

		if (!(enemy & mask)) {
			auto rank = static_cast<int>(sq) / 8;
			bonus += (7 - (white ? (7 - rank) : rank)) * 10;
		}
	}
	return bonus;
}
} // namespace CastleMate::engine