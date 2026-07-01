#pragma once
#include "castlemate/utils/bit_math.hpp"
#include <array>
#include <random>
#include <thread>
#include <vector>

namespace CastleMate {
constexpr auto rook_mask(int sq) -> std::uint64_t {
	auto mask = std::uint64_t{};
	auto rank = sq / 8;
	auto file = sq % 8;

	for (auto f = 1; f < 7; f++) {
		if (f != file) { mask |= 1ULL << ((rank * 8) + f); }
	}

	for (auto r = 1; r < 7; r++) {
		if (r != rank) { mask |= 1ULL << ((r * 8) + file); }
	}

	return mask;
}

constexpr auto bishop_mask(int sq) -> std::uint64_t {
	auto mask = std::uint64_t{};
	auto rank = sq / 8;
	auto file = sq % 8;

	for (auto r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) { mask |= 1ULL << ((r * 8) + f); }
	for (auto r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) { mask |= 1ULL << ((r * 8) + f); }
	for (auto r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) { mask |= 1ULL << ((r * 8) + f); }
	for (auto r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) { mask |= 1ULL << ((r * 8) + f); }

	return mask;
}

constexpr auto blocker_subset(std::size_t index, std::uint64_t mask) -> std::uint64_t {
	auto subset = std::uint64_t{};
	auto temp = mask;
	for (auto i = 0; i < 64; i++) {
		if (!temp) { break; }
		auto bit = pop_lsb(temp);
		if (index & (1 << i)) { subset |= 1ULL << bit; }
	}
	return subset;
}

// inefficient ray attacks used for initializing lookup table
constexpr auto rook_attack_ray(int sq, std::uint64_t blockers) -> std::uint64_t {
	auto attacks = std::uint64_t{};
	auto rank = sq / 8;
	auto file = sq % 8;

	for (auto r = rank + 1; r < 8; r++) {
		attacks |= 1ULL << ((r * 8) + file);
		if (blockers & (1ULL << ((r * 8) + file))) { break; }
	}

	for (auto r = rank - 1; r >= 0; r--) {
		attacks |= 1ULL << ((r * 8) + file);
		if (blockers & (1ULL << ((r * 8) + file))) { break; }
	}

	for (auto f = file + 1; f < 8; f++) {
		attacks |= 1ULL << ((rank * 8) + f);
		if (blockers & (1ULL << ((rank * 8) + f))) { break; }
	}

	for (auto f = file - 1; f >= 0; f--) {
		attacks |= 1ULL << ((rank * 8) + f);
		if (blockers & (1ULL << ((rank * 8) + f))) { break; }
	}

	return attacks;
}

constexpr auto bishop_attack_ray(int sq, std::uint64_t blockers) -> std::uint64_t {
	auto attacks = std::uint64_t{};
	auto rank = sq / 8;
	auto file = sq % 8;

	for (auto r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
		attacks |= 1ULL << ((r * 8) + f);
		if (blockers & (1ULL << ((r * 8) + f))) { break; }
	}

	for (auto r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
		attacks |= 1ULL << ((r * 8) + f);
		if (blockers & (1ULL << ((r * 8) + f))) { break; }
	}

	for (auto r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
		attacks |= 1ULL << ((r * 8) + f);
		if (blockers & (1ULL << ((r * 8) + f))) { break; }
	}

	for (auto r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
		attacks |= 1ULL << ((r * 8) + f);
		if (blockers & (1ULL << ((r * 8) + f))) { break; }
	}

	return attacks;
}

// magic
constexpr auto find_magic(int sq, std::uint64_t mask, bool diagonal) -> std::uint64_t {
	auto bits = std::popcount(mask);
	auto num_subsets = static_cast<std::size_t>(1 << bits);

	auto blockers = std::vector<std::uint64_t>(num_subsets);
	auto attacks = std::vector<std::uint64_t>(num_subsets);

	for (std::size_t i = 0; i < num_subsets; i++) {
		blockers[i] = blocker_subset(i, mask);
		attacks[i] = diagonal ? bishop_attack_ray(sq, blockers[i]) : rook_attack_ray(sq, blockers[i]);
	}

	auto rng = std::mt19937_64{12345};
	while (true) {
		auto magic = rng() & rng() & rng(); // NOLINT
		auto table = std::vector<std::uint64_t>(num_subsets);
		auto failed = false;
		for (std::size_t i = 0; i < num_subsets; i++) {
			auto idx = (blockers[i] * magic) >> (64 - bits);
			if (table[idx] == 0) {
				table[idx] = attacks[i];
			} else if (table[idx] != attacks[i]) {
				failed = true;
				break;
			}
		}
		if (!failed) { return magic; }
	}
}

struct Magic {
	std::uint64_t mask{};
	std::uint64_t magic{};
	int shift{};
	std::vector<std::uint64_t> table{};
};
constexpr auto init_magics(bool diagonal) -> std::array<Magic, 64> {
	auto magics = std::array<Magic, 64>{};

	auto init_square = [&](int sq) {
		auto& m = magics.at(static_cast<std::size_t>(sq));
		m.mask = diagonal ? bishop_mask(sq) : rook_mask(sq);
		auto bits = std::popcount(m.mask);
		m.shift = 64 - bits;
		m.magic = find_magic(sq, m.mask, diagonal);

		std::size_t num_subsets = 1 << bits;
		m.table.resize(num_subsets);
		for (std::size_t i = 0; i < num_subsets; i++) {
			auto blockers = blocker_subset(i, m.mask);
			auto idx = (blockers * m.magic) >> m.shift;
			m.table[idx] = diagonal ? bishop_attack_ray(sq, blockers) : rook_attack_ray(sq, blockers);
		}
	};

	auto num_threads = std::max(1u, std::thread::hardware_concurrency());
	auto threads = std::vector<std::thread>{};
	threads.reserve(num_threads);

	for (unsigned t = 0; t < num_threads; t++) {
		threads.emplace_back([t, num_threads, &init_square]() {
			for (int sq = static_cast<int>(t); sq < 64; sq += static_cast<int>(num_threads)) { init_square(sq); }
		});
	}

	for (auto& th : threads) { th.join(); }

	return magics;
}
} // namespace CastleMate