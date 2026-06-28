#pragma once
#include "castlemate/state.hpp"
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

	[[nodiscard]] auto get_state_manager() -> StateManager& { return m_state_manager; }

	void close() const { m_should_close = true; }

  private:
	void create_data_loader();

	std::unique_ptr<le::Context> m_context{};
	std::unique_ptr<le::IDataLoader> m_data_loader{};

	StateManager m_state_manager{};

	mutable bool m_should_close{};
};
} // namespace CastleMate