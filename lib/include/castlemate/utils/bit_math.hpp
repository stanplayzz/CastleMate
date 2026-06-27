#pragma once
#include <bit>
#include <cstdint>

namespace CastleMate {
constexpr void set_bit(std::uint64_t& bb, int sq) { bb |= (1ULL << sq); }

constexpr void clear_bit(std::uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }

constexpr auto get_bit(std::uint64_t const& bb, int sq) -> bool { return (bb >> sq) & 1; }

constexpr auto pop_lsb(std::uint64_t& bb) -> std::size_t {
	auto index = static_cast<std::size_t>(std::countr_zero(bb));
	bb &= bb - 1;
	return index;
}

constexpr auto replace_bit(std::uint64_t& bb, int from, int to) {
	clear_bit(bb, from);
	set_bit(bb, to);
}

constexpr auto sq(int rank, int file) -> int { return (rank * 8) + file; }
} // namespace CastleMate