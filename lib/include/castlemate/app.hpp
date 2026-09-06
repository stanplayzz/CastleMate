#pragma once
#include "castlemate/engine/engine.hpp"
#include "castlemate/network/network.hpp"
#include "castlemate/state.hpp"
#include <le2d/context.hpp>
#include <le2d/file_data_loader.hpp>

namespace CastleMate {
class App {
  public:
	App(std::optional<std::string> engine);

	void run();

	[[nodiscard]] auto get_context() const -> le::Context const& { return *m_context; }

	[[nodiscard]] auto get_data_loader() const -> le::IDataLoader const& { return *m_data_loader; }

	[[nodiscard]] auto create_asset_loader() const -> le::AssetLoader {
		return m_context->create_asset_loader(&get_data_loader());
	}

	[[nodiscard]] auto get_state_manager() -> StateManager& { return m_state_manager; }

	[[nodiscard]] auto network() -> Network& { return m_network; }

	[[nodiscard]] auto engine() -> std::unique_ptr<IEngine>& { return m_engine; }

	void close() const { m_should_close = true; }

  private:
	void create_data_loader();

	std::unique_ptr<le::Context> m_context{};
	std::unique_ptr<le::IDataLoader> m_data_loader{};
	std::unique_ptr<IEngine> m_engine{};

	StateManager m_state_manager{};

	mutable bool m_should_close{};

	Network m_network{};
};
} // namespace CastleMate