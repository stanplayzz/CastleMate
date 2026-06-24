#include "castlemate/app.hpp"
#include "castlemate/build_version.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/utils/constants.hpp"
#include "castlemate/utils/conversion.hpp"
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

	m_boardView = std::make_unique<BoardView>(this);
	m_boardView->update_board(static_cast<std::uint64_t const*>(m_board.get_bitboard()));
}

void App::run() {
	while (m_context->is_running()) {
		m_context->next_frame();

		handle_input();

		auto& renderer = m_context->begin_render(theme::background_v);
		renderer.viewport = viewport_v;

		m_boardView->draw(renderer);

		m_context->present();
	}

	auto const waiter = m_context->create_waiter();
}

void App::create_data_loader() {
	auto assets_dir = le::FileDataLoader::upfind("assets", le::util::exe_path());
	if (assets_dir.empty()) { throw std::runtime_error{"Failed to find assets dir"}; }

	m_data_loader = std::make_unique<le::FileDataLoader>(assets_dir);
}

void App::handle_input() {
	for (auto const& e : m_context->event_queue()) {
		if (auto const* mouse = std::get_if<le::event::CursorPos>(&e)) { m_mouse_pos = mouse->window; }
		if (auto const* mouse = std::get_if<le::event::MouseButton>(&e)) {
			if (mouse->button == GLFW_MOUSE_BUTTON_1 & mouse->action == GLFW_RELEASE) {
				m_board.click_square(screen_to_sq(m_mouse_pos), m_boardView->get_square_outline());
			}
		}
	}
}
} // namespace CastleMate