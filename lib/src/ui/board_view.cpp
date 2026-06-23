#include "castlemate/ui/board_view.hpp"
#include "castlemate/app.hpp"
#include "castlemate/viewport.hpp"

namespace CastleMate {
BoardView::BoardView(gsl::not_null<App const*> app) : m_app(app) { create_board(); }

void BoardView::draw(le::IRenderer& renderer) const {
	renderer.set_shader(*m_board_shader);
	m_board.draw(renderer);

	renderer.set_shader(m_app->get_context().get_default_shader());
}

void BoardView::create_board() {
	m_board.create({viewport_v.world_size});

	auto const vert = m_app->get_data_loader().load_spir_v("shaders/board.vert.spv");
	auto const frag = m_app->get_data_loader().load_spir_v("shaders/board.frag.spv");
	if (vert.empty() || frag.empty()) { throw std::runtime_error{"Failed to load shader"}; }
	m_board_shader = m_app->get_context().get_resource_factory().create_shader();
	if (!m_board_shader->load(vert, frag)) { throw std::runtime_error{"Failed to load shader"}; }
}
} // namespace CastleMate