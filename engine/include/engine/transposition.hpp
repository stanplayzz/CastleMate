#pragma once
#include "castlemate/core/move.hpp"

namespace CastleMate::engine {
enum class Bound : std::uint8_t { EXACT, LOWER, UPPER };

struct TTEntry {
	std::uint64_t key{};
	std::int32_t score{};
	std::uint8_t depth{};
	Bound bound{};
	Move move{};
};

class TranspositionTable {
  public:
	TranspositionTable() {
		std::size_t bytes = 64 * 1024 * 1024;
		auto count = bytes / sizeof(TTEntry);
		auto pow = std::bit_floor(count);
		m_table.assign(pow, TTEntry{});
		m_size_mask = pow - 1;
	}

	void store(uint64_t key, int depth, int score, Bound bound, Move best) {
		auto& e = m_table[key & m_size_mask];
		if (e.key != key || depth >= e.depth) {
			e = {.key = key, .score = score, .depth = static_cast<std::uint8_t>(depth), .bound = bound, .move = best};
		}
	}

	auto probe(uint64_t key) -> TTEntry* {
		auto& e = m_table[key & m_size_mask];
		return (e.key == key) ? &e : nullptr;
	}

	void clear() { std::ranges::fill(m_table, TTEntry{}); }

  private:
	std::vector<TTEntry> m_table{};
	std::size_t m_size_mask{};
};

constexpr auto infinity_v = 1'000'000;
constexpr auto mate_value_v = infinity_v - 1;
constexpr auto mate_score(int ply) -> int { return mate_value_v - ply; }

constexpr auto score_to_tt(int score, int ply) -> int {
	if (score >= mate_value_v - 1000) { return score + ply; }
	if (score <= -(mate_value_v - 1000)) { return score - ply; }
	return score;
}

constexpr auto score_from_tt(int score, int ply) -> int {
	if (score >= mate_value_v - 1000) { return score - ply; }
	if (score <= -(mate_value_v - 1000)) { return score + ply; }
	return score;
}
} // namespace CastleMate::engine