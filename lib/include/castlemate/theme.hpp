#pragma once
#include <djson/json.hpp>
#include <kvf/color.hpp>
#include <le2d/data_loader.hpp>
#include <gsl/pointers>
#include <iostream>
#include <print>

namespace CastleMate {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Theme {
  public:
	Theme(Theme const&) = delete;
	Theme& operator=(Theme const&) = delete;

	static void initialize(le::IDataLoader& data_loader) {
		if (!s_instance) { s_instance = new Theme(data_loader); }
	}

	template <typename Type>
	static Type from_name(std::initializer_list<std::string_view> path) {
		if (!s_instance || !s_instance->m_json) {
			std::println(std::cerr, "Tried to retrieve json object before initialization.");
			return {};
		}
		if (std::is_same_v<Type, kvf::Color>) { return s_instance->load_color(path); }

		std::println(std::cerr, "Unknown Type");
	}

  private:
	Theme(le::IDataLoader& data_loader) {
		m_json = data_loader.load_json("config/theme.json");
		if (!m_json) { throw std::runtime_error{"Failed to load theme.json"}; }
		std::println("[I] [Theme] initialized");
	}

	auto load_color(std::initializer_list<std::string_view> path) -> kvf::Color {
		auto const* current = &m_json;
		for (auto const& key : path) { current = &((*current)[key]); }
		auto const& arr = *current;
		return {glm::vec4{arr[0].as<float>(), arr[1].as<float>(), arr[2].as<float>(), arr[3].as<float>()}};
	}

	dj::Json m_json{};
	inline static gsl::owner<Theme*> s_instance = nullptr;
};
} // namespace CastleMate