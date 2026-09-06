#pragma once
#include "castlemate/engine/engine.hpp"
#include <filesystem>
#include <mutex>
#include "reproc++/reproc.hpp"

namespace CastleMate {
class UciEngine : public IEngine { // NOLINT
  public:
	explicit UciEngine(std::filesystem::path const& engine_binary);
	~UciEngine();

	void new_game() override;
	void set_position(Position const& position) override;
	void go(engine::SearchParams params) override;
	void stop() override;

	void on_best_move(std::function<void(Move)> cb) override { m_on_best_move = std::move(cb); }
	void on_info(std::function<void(std::string)> cb) override { m_on_info = std::move(cb); }

  private:
	void send(std::string const& cmd);
	void reader_loop(std::stop_token const& token);
	void handle_line(std::string const& line);

	std::mutex m_position_mutex{};
	Position m_position{};

	std::function<void(Move)> m_on_best_move{};
	std::function<void(std::string)> m_on_info{};

	reproc::process m_process{};
	std::jthread m_reader_thread{};
};
} // namespace CastleMate