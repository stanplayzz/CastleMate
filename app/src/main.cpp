#include <castlemate/app.hpp>
#include <castlemate/utils/perft.hpp>
#include <print>

namespace {
constexpr auto debug_options(int argc, char* argv[]) { // NOLINT
	if (argc <= 3) { return; }

	auto const command = std::string_view{argv[1]}; // NOLINT
	auto const arg1 = std::string_view{argv[2]};	// NOLINT
	auto const arg2 = std::string_view{argv[3]};	// NOLINT

	constexpr auto depth_prefix_v = std::string_view{"--depth="};
	constexpr auto fen_prefix_v = std::string_view{"--fen="};

	if (command == "--perft" && arg1.starts_with(depth_prefix_v) && arg2.starts_with(fen_prefix_v)) {
		auto const depth_str = arg1.substr(depth_prefix_v.size());
		auto depth = int{};
		auto const [ptr, ec] = std::from_chars(depth_str.data(), depth_str.data() + depth_str.size(), depth); // NOLINT

		if (ec != std::errc{} || depth < 1) {
			throw std::runtime_error{std::format("Invalid --depth value: {}", depth_str)};
		}

		auto const fen = arg2.substr(fen_prefix_v.size());
		std::println("Running perft with depth {} on FEN: {}", depth, fen);

		auto pos = CastleMate::Position::from_fen(fen);

		for (auto d = 1; d <= depth; d++) { std::println("Depth {} = {} Nodes", d, CastleMate::engine::perft(pos, d)); }

		std::println("");
		CastleMate::engine::perft_divide(pos, depth);
	}
}

auto parse_engine_arg(int argc, char* argv[]) -> std::optional<std::string> { // NOLINT
	constexpr auto engine_prefix_v = std::string_view{"--engine="};

	for (int i = 1; i < argc; ++i) {
		auto const arg = std::string_view{argv[i]}; // NOLINT
		if (arg.starts_with(engine_prefix_v)) { return std::string{arg.substr(engine_prefix_v.size())}; }
	}
	return std::nullopt;
}
} // namespace

auto main(int argc, char* argv[]) -> int {
	debug_options(argc, argv);

	try {
		CastleMate::App{parse_engine_arg(argc, argv)}.run();
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}