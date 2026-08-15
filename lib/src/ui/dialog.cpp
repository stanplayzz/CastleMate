#include "castlemate/ui/dialog.hpp"

namespace CastleMate::ui {
namespace {
constexpr auto spacing_v = 16.f;
constexpr auto padding_v = 32.f;
constexpr auto white_promo_v = std::array{WR, WN, WB, WQ};
constexpr auto black_promo_v = std::array{BR, BN, BB, BQ};
constexpr auto type_map_v = std::array<int, 6>{5, 3, 2, 4, 1, 0};
constexpr auto cell_w_v = 1.0f / 6.0f;
constexpr auto cell_h_v = 1.0f / 2.0f;

[[nodiscard]] auto atlas_uv(Piece piece) -> kvf::UvRect {
	auto const row = static_cast<float>(piece >= BP);
	auto const col = static_cast<float>(type_map_v.at(static_cast<std::size_t>(piece % 6)));
	auto const lt = glm::vec2{col * cell_w_v, row * cell_h_v};
	return {.lt = lt, .rb = lt + glm::vec2{cell_w_v, cell_h_v}};
}
} // namespace

SpriteDialogItem::SpriteDialogItem(le::ITexture* texture, glm::vec2 size, kvf::UvRect uv) {
	m_sprite.set_base_size(size);
	m_sprite.set_texture(texture, uv);
}

TextDialogItem::TextDialogItem(le::IFont& font, std::string_view label, le::TextHeight height, kvf::Color tint) {
	m_text.set_string(font, std::string{label}, {.height = height});
	m_text.tint = tint;
}

void Dialog::open(std::vector<std::unique_ptr<IDialogItem>> items, IDialogItem* label) {
	m_items = std::move(items);
	m_label = label;

	auto row_width = 0.0f;
	auto row_height = 0.0f;
	for (auto const& item : m_items) {
		auto const size = item->bounds().size();
		row_width += size.x;
		row_height = std::max(row_height, size.y);
	}
	row_width += spacing_v * static_cast<float>(m_items.size() - 1);

	auto const label_size = m_label != nullptr ? m_label->bounds().size() : glm::vec2{};
	auto const total_width = std::max(row_width, label_size.x);
	auto const total_height = row_height + (m_label != nullptr ? label_size.y + spacing_v : 0.0f);

	m_widget.create({total_width + (padding_v * 2.0f), total_height + (padding_v * 2.0f)});

	auto const row_y = m_label != nullptr ? -(total_height * 0.5f) + (row_height * 0.5f) : 0.0f;
	if (m_label != nullptr) { m_label->set_position({0.0f, (total_height * 0.5f) - (label_size.y * 0.5f)}); }

	auto x = -total_width * 0.5f;
	for (auto const& item : m_items) {
		auto const width = item->bounds().size().x;
		item->set_position({x + (width * 0.5f), row_y});
		x += width + spacing_v;
	}

	m_open = true;
}

auto Dialog::click(glm::vec2 pos) const -> std::optional<int> {
	if (!m_open) { return std::nullopt; }
	for (std::size_t i = 0; i < m_items.size(); ++i) {
		if (m_items[i]->bounds().contains(pos)) { return static_cast<int>(i); }
	}
	return std::nullopt;
}

PromotionDialog::PromotionDialog(gsl::not_null<App const*> app) : m_dialog(app) {
	m_piece_texture = app->create_asset_loader().load<le::ITexture>("images/piece_atlas.png");
	if (!m_piece_texture) { throw std::runtime_error{"Failed to load texture"}; }
}

void PromotionDialog::open(bool white) {
	m_white = white;
	auto const& pieces = white ? white_promo_v : black_promo_v;
	constexpr auto sprite_size = glm::vec2{256.0f};

	auto items = std::vector<std::unique_ptr<IDialogItem>>{};
	items.reserve(pieces.size());
	for (auto const piece : pieces) {
		items.push_back(std::make_unique<SpriteDialogItem>(m_piece_texture.get(), sprite_size, atlas_uv(piece)));
	}
	m_dialog.open(std::move(items));
}

auto PromotionDialog::click(glm::vec2 pos) const -> std::optional<Piece> {
	auto const index = m_dialog.click(pos);
	if (!index) { return std::nullopt; }
	auto const& pieces = m_white ? white_promo_v : black_promo_v;
	return pieces.at(static_cast<std::size_t>(*index));
}

ConfirmDialog::ConfirmDialog(gsl::not_null<App const*> app, le::IFont& font) : m_dialog(app), m_font(font) {}

void ConfirmDialog::open(std::string_view text) {
	m_text = std::make_unique<TextDialogItem>(m_font, text, le::TextHeight{64});

	auto items = std::vector<std::unique_ptr<IDialogItem>>{};
	items.push_back(std::make_unique<TextDialogItem>(m_font, "YES", le::TextHeight{64}, kvf::black_v));
	items.push_back(std::make_unique<TextDialogItem>(m_font, "NO", le::TextHeight{64}, kvf::black_v));
	m_dialog.open(std::move(items), m_text.get());
}

void ConfirmDialog::draw(le::IRenderer& renderer) const { m_dialog.draw(renderer); }

auto ConfirmDialog::click(glm::vec2 pos) const -> std::optional<bool> {
	auto const index = m_dialog.click(pos);
	if (!index) { return std::nullopt; }
	return *index == 0;
}
} // namespace CastleMate::ui