#pragma once
#include "castlemate/app.hpp"
#include "castlemate/core/piece.hpp"
#include "castlemate/ui/widget.hpp"
#include <le2d/drawable/sprite.hpp>
#include <le2d/drawable/text.hpp>

namespace CastleMate::ui {
class IDialogItem : klib::Polymorphic { // NOLINT
  public:
	virtual ~IDialogItem() = default;
	virtual void draw(le::IRenderer& renderer) const = 0;
	virtual void set_position(glm::vec2 pos) = 0;
	[[nodiscard]] virtual auto bounds() const -> kvf::Rect<float> = 0;
};

class Dialog {
  public:
	explicit Dialog(gsl::not_null<App const*> app) : m_app(app) {}

	void open(std::vector<std::unique_ptr<IDialogItem>> items, IDialogItem* label = nullptr);
	void close() { m_open = false; };

	void draw(le::IRenderer& renderer) const {
		m_widget.draw(renderer);
		if (m_label != nullptr) { m_label->draw(renderer); }
		for (auto const& item : m_items) { item->draw(renderer); }
	}

	[[nodiscard]] auto is_open() const -> bool { return m_open; }
	[[nodiscard]] auto click(glm::vec2 pos) const -> std::optional<int>;

  private:
	gsl::not_null<App const*> m_app;

	Widget m_widget{};
	std::vector<std::unique_ptr<IDialogItem>> m_items{};
	IDialogItem* m_label{};

	bool m_open{};
};

class SpriteDialogItem final : public IDialogItem {
  public:
	SpriteDialogItem(le::ITexture* texture, glm::vec2 size, kvf::UvRect = {});

	void draw(le::IRenderer& renderer) const override { m_sprite.draw(renderer); }

	void set_position(glm::vec2 pos) override { m_sprite.transform.position = pos; }

	[[nodiscard]] auto bounds() const -> kvf::Rect<float> override { return m_sprite.bounding_rect(); }

  private:
	le::drawable::Sprite m_sprite;
};

class TextDialogItem final : public IDialogItem {
  public:
	TextDialogItem(le::IFont& font, std::string_view label, le::TextHeight height = {}, kvf::Color tint = kvf::black_v);

	void draw(le::IRenderer& renderer) const override { m_text.draw(renderer); }

	void set_position(glm::vec2 pos) override { m_text.transform.position = pos; }

	[[nodiscard]] auto bounds() const -> kvf::Rect<float> override {
		auto ret = kvf::Rect<float>{
			.lt = m_text.transform.position,
			.rb = m_text.transform.position,
		};
		ret.lt.x -= m_text.get_size().x * 0.5f;
		ret.lt.y += m_text.get_size().y;
		ret.rb.x += m_text.get_size().x * 0.5f;
		return ret;
	}

  private:
	le::drawable::Text m_text;
};

class PromotionDialog {
  public:
	explicit PromotionDialog(gsl::not_null<App const*> app);

	void open(bool white);
	void close() { m_dialog.close(); }

	void draw(le::IRenderer& renderer) const { m_dialog.draw(renderer); }

	[[nodiscard]] auto is_open() const -> bool { return m_dialog.is_open(); }
	[[nodiscard]] auto click(glm::vec2 pos) const -> std::optional<Piece>;

  private:
	bool m_white{};

	Dialog m_dialog;
	std::unique_ptr<le::ITexture> m_piece_texture;
};

class ConfirmDialog {
  public:
	ConfirmDialog(gsl::not_null<App const*> app, le::IFont& font);

	void open(std::string_view text);
	void close() { m_dialog.close(); }

	void draw(le::IRenderer& renderer) const;

	[[nodiscard]] auto is_open() const -> bool { return m_dialog.is_open(); }
	[[nodiscard]] auto click(glm::vec2 pos) const -> std::optional<bool>;

  private:
	Dialog m_dialog;
	std::unique_ptr<TextDialogItem> m_text{};
	le::IFont& m_font;
};
} // namespace CastleMate::ui