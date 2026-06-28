#include "castlemate/app.hpp"
#include "castlemate/build_version.hpp"
#include "castlemate/states/gameplay.hpp"
#include "castlemate/states/menu.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/utils/constants.hpp"
#include <le2d/util.hpp>

namespace CastleMate {
namespace {
auto const window_title_v = std::format("CastleMate {}", buildVersionStr);
auto const context_create_info_v = le::Context::CreateInfo{
	.window = le::WindowInfo{.size = {512, 512}, .title = window_title_v},
};
} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
App::App() {
	m_context = le::Context::create(context_create_info_v);
	create_data_loader();
	Theme::initialize(*m_data_loader);
}

void App::run() {
	m_state_manager.set_state(std::make_unique<Menu>(this));

	while (m_context->is_running()) {
		m_context->next_frame();

		m_state_manager.update();

		auto& renderer = m_context->begin_render(m_state_manager.get_clear_color());
		renderer.viewport = viewport_v;

		m_state_manager.draw(renderer);

		m_context->present();

		if (m_should_close) { break; }
	}

	auto const waiter = m_context->create_waiter();
}

void App::create_data_loader() {
	auto assets_dir = le::FileDataLoader::upfind("assets", le::util::exe_path());
	if (assets_dir.empty()) { throw std::runtime_error{"Failed to find assets dir"}; }

	m_data_loader = std::make_unique<le::FileDataLoader>(assets_dir);
}
} // namespace CastleMate