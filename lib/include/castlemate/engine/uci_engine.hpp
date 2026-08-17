#pragma once
#include "castlemate/core/move.hpp"
#include <klib/base_types.hpp>
#include <filesystem>
#include <functional>
#include <mutex>
#include "reproc++/reproc.hpp"

namespace CastleMate {
class UciEngine : public klib::MoveOnly { // NOLINT
  public:
	explicit UciEngine(std::filesystem::path const& engine_binary);
	~UciEngine();

	void new_game();
	void set_position(Position const& position);
	void go();
	void stop();

	void on_best_move(std::function<void(Move)> cb) { m_on_best_move = std::move(cb); }
	void on_info(std::function<void(std::string)> cb) { m_on_info = std::move(cb); }

  private:
	void send(std::string const& cmd);
	void reader_loop(std::stop_token const& token);
	void handle_line(std::string const& line);

	reproc::process m_process{};
	std::jthread m_reader_thread{};

	std::mutex m_position_mutex{};
	Position m_position{};

	std::function<void(Move)> m_on_best_move{};
	std::function<void(std::string)> m_on_info{};
};
} // namespace CastleMate