#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>

namespace shared {
namespace {
constexpr auto host_to_big_endian(std::uint64_t value) -> std::uint64_t {
	if constexpr (std::endian::native == std::endian::little) {
		return std::byteswap(value);
	} else {
		return value;
	}
}

constexpr auto big_endian_to_host(std::uint64_t value) -> std::uint64_t { return host_to_big_endian(value); }
} // namespace

enum class MsgType : std::uint8_t {
	JoinQueue = 0x01,
	LeaveQueue = 0x02,
	MatchFound = 0x03,
	Move = 0x04,
	Resign = 0x05,
	DrawOffer = 0x06,
	DrawAccept = 0x07,
	GameOver = 0x08,
};

struct MatchFoundMsg {
	std::uint64_t game_id{};
	bool white{};
};

inline auto match_found_to_bytes(MatchFoundMsg const& msg) -> std::array<std::byte, 10> {
	auto buffer = std::array<std::byte, 10>{};
	buffer.at(0) = std::byte{std::to_underlying(MsgType::MatchFound)};
	auto game_id = host_to_big_endian(msg.game_id);
	std::memcpy(buffer.data() + 1, &game_id, 8); // NOLINT
	buffer.at(9) = std::byte{msg.white};
	return buffer;
}

inline auto bytes_to_match_found(std::span<std::byte const> data) -> MatchFoundMsg {
	if (data.size() < 10) { return {}; }
	auto game_id = std::uint64_t{};
	std::memcpy(&game_id, data.data() + 1, 8); // NOLINT
	return {.game_id = big_endian_to_host(game_id), .white = static_cast<bool>(data[9])};
}

enum class GameOverReason : std::uint8_t { Resign, Draw, Checkmate };

struct GameOverMsg {
	GameOverReason reason{};
	bool white_won{};
};

inline auto game_over_to_bytes(GameOverMsg const& msg) -> std::array<std::byte, 3> {
	std::array<std::byte, 3> buffer{};
	buffer[0] = std::byte{std::to_underlying(MsgType::GameOver)};
	buffer[1] = std::byte{std::to_underlying(msg.reason)};
	buffer[2] = std::byte{msg.white_won};
	return buffer;
}

inline auto bytes_to_game_over(std::span<std::byte const> data) -> GameOverMsg {
	if (data.size() < 3) { return {}; }
	return GameOverMsg{
		.reason = static_cast<GameOverReason>(data[1]),
		.white_won = static_cast<bool>(data[2]),
	};
}
} // namespace shared