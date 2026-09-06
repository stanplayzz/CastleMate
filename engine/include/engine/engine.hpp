#pragma once
#include "castlemate/core/position.hpp"
#include "engine/search_context.hpp"
#include <iostream>

namespace CastleMate::engine {
class Engine {
  public:
	Engine(std::istream& in = std::cin, std::ostream& out = std::cout) : m_in(in), m_out(out) {}

	void run();

	void handle_command(std::string const& line);

  private:
	void new_game();

	std::istream& m_in;
	std::ostream& m_out;

	Position m_position{};
	SearchContext m_context{};
};
} // namespace CastleMate::engine