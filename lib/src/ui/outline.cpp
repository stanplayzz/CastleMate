#include "castlemate/ui/outline.hpp"
#include "castlemate/app.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/utils/constants.hpp"

namespace CastleMate {
SquareOutline::SquareOutline(gsl::not_null<App const*> app) : m_app(app) {
	m_quad.create(tile_size_v);
	m_quad.tint = Theme::from_name<kvf::Color>({"square_highlight"});

	auto const vert = m_app->get_data_loader().load_spir_v("shaders/outline.vert.spv");
	auto const frag = m_app->get_data_loader().load_spir_v("shaders/outline.frag.spv");
	if (vert.empty() || frag.empty()) { throw std::runtime_error{"Failed to load shader"}; }
	m_shader = m_app->get_context().get_resource_factory().create_shader();
	if (!m_shader->load(vert, frag)) { throw std::runtime_error{"Failed to load shader"}; }
}

void SquareOutline::draw(le::IRenderer& renderer) const {
	renderer.set_shader(*m_shader);
	m_quad.draw(renderer);
	renderer.set_shader(m_app->get_context().get_default_shader());
}

void SquareOutline::set_position(glm::vec2 pos) { m_quad.transform.position = pos; }
} // namespace CastleMate