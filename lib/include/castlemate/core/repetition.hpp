#pragma once
#include "castlemate/core/position.hpp"

namespace CastleMate {
inline auto is_repetition(Position const& pos) -> bool {
	auto const* state = pos.state ? pos.state->previous : nullptr;
	auto matches = 0;
	while (state) {
		if (state->hash == pos.hash) {
			if (++matches == 2) { return true; }
		}
		if (state->irreversible) { break; }
		state = state->previous;
	}
	return false;
}

inline auto is_fifty_move_draw(Position const& pos) -> bool { return pos.halfmove_clock >= 100; }

inline auto is_draw(Position const& pos) -> bool { return is_repetition(pos) || is_fifty_move_draw(pos); }

} // namespace CastleMate