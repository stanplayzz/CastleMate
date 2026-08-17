#pragma once
#include "castlemate/core/position.hpp"

namespace CastleMate::engine {
class Engine {
  public:
	Engine();

  private:
	void new_game();

	Position m_position{};
};
} // namespace CastleMate::engine