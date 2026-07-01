#include "castlemate/ui/board_view.hpp"
#include "castlemate/app.hpp"
#include "castlemate/theme.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "castlemate/utils/constants.hpp"

namespace CastleMate {
BoardView::BoardView(gsl::not_null<App const*> app) : m_app(app) {
	create_board();
	load_piece_texture();
	m_square_outline = std::make_unique<SquareOutline>(app);

	m_font = app->create_asset_loader().load<le::IFont>("fonts/CormorantGaramond.ttf");
	if (!m_font) { throw std::runtime_error{"Failed to load font"}; }
}

void BoardView::draw(le::IRenderer& renderer) const {
	renderer.set_shader(*m_board_shader);

	struct BoardColors {
		glm::vec4 light{};
		glm::vec4 dark{};
	} colors;

	colors.light = Theme::from_name<kvf::Color>({"board", "light"}).to_vec4();
	colors.dark = Theme::from_name<kvf::Color>({"board", "dark"}).to_vec4();
	renderer.set_push_constants(vk::ShaderStageFlagBits::eFragment, sizeof(BoardColors), &colors);
	m_board.draw(renderer);

	renderer.set_shader(m_app->get_context().get_default_shader());

	for (auto const& p : m_piece_sprites) { p.draw(renderer); }

	m_square_outline->draw(renderer);

	if (m_show_promotion) {
		m_promotion_ui.border.draw(renderer);
		m_promotion_ui.background.draw(renderer);
		for (auto const& c : m_promotion_ui.choices) { c.draw(renderer); }
	}

	m_end_text.draw(renderer);
	m_end_sub_text.draw(renderer);
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
void BoardView::update_board(std::uint64_t const* bitboards, bool white_bottom) {
	m_pieces.fill(COUNT_);
	auto bb = bitboards[WP];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WP; }
	bb = bitboards[WR];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WR; }
	bb = bitboards[WN];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WN; }
	bb = bitboards[WB];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WB; }
	bb = bitboards[WQ];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WQ; }
	bb = bitboards[WK];
	while (bb) { m_pieces.at(pop_lsb(bb)) = WK; }

	bb = bitboards[BP];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BP; }
	bb = bitboards[BR];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BR; }
	bb = bitboards[BN];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BN; }
	bb = bitboards[BB];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BB; }
	bb = bitboards[BQ];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BQ; }
	bb = bitboards[BK];
	while (bb) { m_pieces.at(pop_lsb(bb)) = BK; }

	update_pieces(white_bottom);
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

void BoardView::end_game(GameEnding ending) {
	auto string = std::string{};
	if (ending.draw) {
		string = "DRAW!";
	} else {
		string = ending.white_won ? "WHITE WON!" : "BLACK WON!";
	}

	m_end_text.set_string(*m_font, string, {.height = le::TextHeight{70}});
	m_end_text.tint = Theme::from_name<kvf::Color>({"end_text"});
	m_end_sub_text.set_string(*m_font, "PRESS ANY BUTTON TO RETURN", {.height = le::TextHeight{30}});
	m_end_sub_text.tint = Theme::from_name<kvf::Color>({"end_text"});
	m_end_sub_text.transform.position.y = -50;
}

void BoardView::create_board() {
	m_board.create({viewport_v.world_size});

	auto const vert = m_app->get_data_loader().load_spir_v("shaders/board.vert.spv");
	auto const frag = m_app->get_data_loader().load_spir_v("shaders/board.frag.spv");
	if (vert.empty() || frag.empty()) { throw std::runtime_error{"Failed to load shader"}; }
	m_board_shader = m_app->get_context().get_resource_factory().create_shader();
	if (!m_board_shader->load(vert, frag)) { throw std::runtime_error{"Failed to load shader"}; }
}

void BoardView::load_piece_texture() {
	m_piece_texture = m_app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }
}

void BoardView::update_pieces(bool white_bottom) {
	if (m_pieces.empty()) { return; }
	m_piece_sprites.clear();

	for (std::size_t i = 0; i < m_pieces.size(); ++i) {
		auto& p = m_pieces.at(i);
		if (p == COUNT_) { continue; }
		// map Piece to atlas
		constexpr auto type_map = std::array<int, 6>{5, 3, 2, 4, 1, 0};

		int row = (p >= BP);
		int col = type_map.at(static_cast<std::size_t>(p % 6));

		constexpr auto cellW = 1.f / 6.f;
		constexpr auto cellH = 1.f / 2.f;

		auto uvMin = glm::vec2{static_cast<float>(col) * cellW, static_cast<float>(row) * cellH};
		auto uvMax = glm::vec2{uvMin.x + cellW, uvMin.y + cellH};

		m_piece_sprites.emplace_back(glm::vec2{tile_size_v}).set_texture(m_piece_texture.get());
		m_piece_sprites.back().set_uv({.lt = uvMin, .rb = uvMax});

		auto file = static_cast<float>(i & 7);
		auto rank = static_cast<float>(i >> 3);

		if (!white_bottom) {
			file = 7 - file;
			rank = 7 - rank;
		}

		auto pos = glm::vec2{file * tile_size_v.x, rank * tile_size_v.x};
		pos -= viewport_v.world_size * 0.5f - glm::vec2{tile_size_v * 0.5f};

		m_piece_sprites.back().transform.position = pos;
	}
}

void BoardView::create_promotion(bool white) {
	auto const pieces = white ? std::array{WR, WN, WB, WQ} : std::array{BR, BN, BB, BQ};

	auto sprite_size = glm::vec2{viewport_v.world_size.y * 0.14f};

	for (std::size_t i = 0; i < 4; i++) {
		constexpr auto type_map = std::array<int, 6>{5, 3, 2, 4, 1, 0};

		int row = (pieces.at(i) >= BP);
		int col = type_map.at(static_cast<std::size_t>(pieces.at(i) % 6));

		constexpr auto cellW = 1.f / 6.f;
		constexpr auto cellH = 1.f / 2.f;

		auto uvMin = glm::vec2{static_cast<float>(col) * cellW, static_cast<float>(row) * cellH};
		auto uvMax = glm::vec2{uvMin.x + cellW, uvMin.y + cellH};
		m_promotion_ui.choices.at(i).set_base_size(sprite_size);
		m_promotion_ui.choices.at(i).set_texture(m_piece_texture.get());
		m_promotion_ui.choices.at(i).set_uv({.lt = uvMin, .rb = uvMax});
		m_promotion_ui.choices.at(i).transform.position = {
			(sprite_size.x * -1.5f) + (static_cast<float>(i) * sprite_size.x), 0};
	}

	m_promotion_ui.background.create(glm::vec2{sprite_size.x * 4.f, sprite_size.y});
	m_promotion_ui.background.tint = Theme::from_name<kvf::Color>({"ui_background"});
	m_promotion_ui.border.create(glm::vec2{sprite_size.x * 4.f, sprite_size.y} + sprite_size * 0.1f);
	m_promotion_ui.border.tint = Theme::from_name<kvf::Color>({"ui_border"});
}
} // namespace CastleMate