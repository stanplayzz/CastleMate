#pragma once
#include "castlemate/engine/engine.hpp"
#include "engine/engine.hpp"
#include <sstream>

namespace CastleMate {
class NativeEngine : public IEngine { // NOLINT
  public:
	void new_game() override;
	void set_position(Position const& position) override;
	void go(engine::SearchParams params) override;
	void stop() override;

	void on_best_move(std::function<void(Move)> cb) override { m_on_best_move = std::move(cb); }
	void on_info(std::function<void(std::string)> cb) override { m_on_info = std::move(cb); }

  private:
	void send(std::string const& cmd);
	void handle_line(std::string const& line);

	std::istringstream m_in{};
	std::ostringstream m_out{};
	engine::Engine m_engine{m_in, m_out};

	Position m_position{};

	std::function<void(Move)> m_on_best_move{};
	std::function<void(std::string)> m_on_info{};
};
} // namespace CastleMate