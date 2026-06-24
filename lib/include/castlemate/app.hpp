#pragma once
#include "castlemate/core/board.hpp"
#include "castlemate/ui/board_view.hpp"
#include <le2d/context.hpp>
#include <le2d/file_data_loader.hpp>

namespace CastleMate {
class App {
  public:
	App();

	void run();

	[[nodiscard]] auto get_context() const -> le::Context const& { return *m_context; }

	[[nodiscard]] auto get_data_loader() const -> le::IDataLoader const& { return *m_data_loader; }

	[[nodiscard]] auto create_asset_loader() const -> le::AssetLoader {
		return m_context->create_asset_loader(&get_data_loader());
	}

	[[nodiscard]] auto get_board() const -> Board const& { return m_board; }

  private:
	void create_data_loader();
	void handle_input();

	std::unique_ptr<le::Context> m_context{};
	std::unique_ptr<le::IDataLoader> m_data_loader{};

	Board m_board{};
	std::unique_ptr<BoardView> m_boardView{};

	// input
	glm::vec2 m_mouse_pos{};
};
} // namespace CastleMate