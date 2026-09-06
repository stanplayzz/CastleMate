#pragma once
#include "castlemate/core/move.hpp"
#include "castlemate/engine/search_params.hpp"
#include <klib/base_types.hpp>
#include <functional>

namespace CastleMate {
class IEngine : public klib::Polymorphic {
  public:
	virtual void new_game() = 0;
	virtual void set_position(Position const& position) = 0;
	virtual void go(engine::SearchParams params = {}) = 0;
	virtual void stop() = 0;

	virtual void on_best_move(std::function<void(Move)> cb) = 0;
	virtual void on_info(std::function<void(std::string)> cb) = 0;
};
} // namespace CastleMate