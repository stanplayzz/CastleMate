#include "castlemate/ui/captured_pieces.hpp"
#include <algorithm>

namespace CastleMate::ui {
void CapturedPieces::add(Piece piece) {
	constexpr auto type_map = std::array<int, 6>{5, 3, 2, 4, 1, 0};

	int row = (piece >= BP);
	int col = type_map.at(static_cast<std::size_t>(piece % 6));

	constexpr auto cellW = 1.f / 6.f;
	constexpr auto cellH = 1.f / 2.f;

	auto uvMin = glm::vec2{static_cast<float>(col) * cellW, static_cast<float>(row) * cellH};
	auto uvMax = glm::vec2{uvMin.x + cellW, uvMin.y + cellH};

	auto& sprite = (piece < BP) ? m_white_pieces.emplace_back() : m_black_pieces.emplace_back();
	sprite.first.set_base_size(glm::vec2{128});
	sprite.first.set_texture(m_texture, {.lt = uvMin, .rb = uvMax});
	sprite.second = piece;

	order();
}

void CapturedPieces::draw(le::IRenderer& renderer) const {
	for (auto const& sprite : m_white_pieces) { sprite.first.draw(renderer); }
	for (auto const& sprite : m_black_pieces) { sprite.first.draw(renderer); }
}

void CapturedPieces::order() {
	std::ranges::sort(m_white_pieces, [](auto const& a, auto const& b) {
		return a.second < b.second;
	});

	std::ranges::sort(m_black_pieces, [](auto const& a, auto const& b) {
		return a.second < b.second;
	});

	auto cellSize = m_bounds.size().x / 5.f;

	auto layout = [&](auto& pieces, float startY) {
		for (std::size_t i = 0; i < pieces.size(); ++i) {
			auto column = i % 5;
			auto row = i / 5;

			pieces[i].first.transform.position = {m_bounds.lt.x + (cellSize * (0.5f + static_cast<float>(column))),
												  startY - (cellSize * (0.5f + static_cast<float>(row)))};
		}
	};

	layout(m_white_pieces, m_bounds.lt.y);

	layout(m_black_pieces, m_bounds.lt.y + (cellSize * 3.f));
}
} // namespace CastleMate::ui