#include "castlemate/app.hpp"
#include "castlemate/build_version.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/viewport.hpp"
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
	m_board = std::make_unique<Board>(this);
}

void App::run() {
	while (m_context->is_running()) {
		m_context->next_frame();

		auto& renderer = m_context->begin_render(theme::background_v);
		renderer.viewport = viewport_v;

		m_board->draw(renderer);

		m_context->present();
	}

	auto const waiter = m_context->create_waiter();
}

void App::create_data_loader() {
	auto assets_dir = le::FileDataLoader::upfind("assets", le::util::exe_path());
	if (assets_dir.empty()) { throw std::runtime_error{"Failed to find assets dir"}; }

	m_data_loader = std::make_unique<le::FileDataLoader>(assets_dir);
}
} // namespace CastleMate