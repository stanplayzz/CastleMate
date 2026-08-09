#pragma once
#include <cstdint>

namespace shared {
enum class MsgType : std::uint8_t {
	JoinQueue = 0x01,
	LeaveQueue = 0x02,
	Move = 0x04,
	DrawOffer = 0x05,
	Resign = 0x06,
};
}