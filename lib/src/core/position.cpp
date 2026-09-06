#include "castlemate/core/position.hpp"
#include "castlemate/utils/bit_math.hpp"
#include "engine/zobrist.hpp"
#include <array>
#include <sstream>

namespace CastleMate {
namespace {
constexpr auto piece_from_char(char c) -> Piece {
	switch (c) {
	case 'P': return WP;
	case 'N': return WN;
	case 'B': return WB;
	case 'R': return WR;
	case 'Q': return WQ;
	case 'K': return WK;
	case 'p': return BP;
	case 'n': return BN;
	case 'b': return BB;
	case 'r': return BR;
	case 'q': return BQ;
	case 'k': return BK;
	default: throw std::runtime_error{"Invalid FEN piece"};
	}
}
} // namespace

auto Position::from_fen(std::string_view fen) -> Position {
	auto pos = Position{};

	auto stream = std::istringstream{std::string{fen}};

	auto board = std::string{};
	auto turn = std::string{};
	auto castling = std::string{};
	auto en_passant = std::string{};
	auto halfmove = std::string{};
	stream >> board >> turn >> castling >> en_passant >> halfmove;

	auto rank = 7;
	auto file = 0;

	for (char c : board) {
		if (c == '/') {
			--rank;
			file = 0;
			continue;
		}

		if (c >= '1' && c <= '8') {
			file += c - '0';
			continue;
		}

		auto const sq = (rank * 8) + file;

		set_bit(pos.bb[piece_from_char(c)], sq); // NOLINT
		++file;
	}

	if (turn == "w") {
		pos.turn = Color::White;
	} else if (turn == "b") {
		pos.turn = Color::Black;
	} else {
		throw std::runtime_error{"Invalid FEN turn"};
	}

	pos.castle_wk = castling.contains('K');
	pos.castle_wq = castling.contains('Q');
	pos.castle_bk = castling.contains('k');
	pos.castle_bq = castling.contains('q');

	if (en_passant != "-") {
		auto const file = en_passant.at(0) - 'a';
		auto const rank = en_passant.at(1) - '1';

		pos.en_passant = (rank * 8) + file;
	}

	pos.white_occ = pos.bb[WP] | pos.bb[WR] | pos.bb[WN] | pos.bb[WB] | pos.bb[WQ] | pos.bb[WK];
	pos.black_occ = pos.bb[BP] | pos.bb[BR] | pos.bb[BN] | pos.bb[BB] | pos.bb[BQ] | pos.bb[BK];
	pos.occ = pos.white_occ | pos.black_occ;

	pos.halfmove_clock = halfmove.empty() ? 0 : std::stoi(halfmove);

	pos.hash = engine::compute_hash(pos);

	return pos;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Position::to_fen() const -> std::string {
	auto fen = std::string{};

	constexpr auto piece_chars_v = std::array{
		'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k',
	};

	for (auto rank = 7; rank >= 0; --rank) {
		auto empty = 0;
		for (auto file = 0; file < 8; ++file) {
			auto const sq = (rank * 8) + file;
			auto piece = Piece::COUNT_;

			for (auto i = 0; i < static_cast<int>(Piece::COUNT_); ++i) {
				auto const p = static_cast<Piece>(i);
				if (get_bit(bb[p], sq)) { // NOLINT
					piece = p;
					break;
				}
			}

			if (piece == Piece::COUNT_) {
				++empty;
				continue;
			}

			if (empty > 0) {
				fen += std::to_string(empty);
				empty = 0;
			}

			fen += piece_chars_v.at(piece);
		}

		if (empty > 0) { fen += std::to_string(empty); }
		if (rank > 0) { fen += '/'; }
	}

	fen += turn == Color::White ? " w " : " b ";

	if (!castle_wk && !castle_wq && !castle_bk && !castle_bq) {
		fen += '-';
	} else {
		if (castle_wk) { fen += 'K'; }
		if (castle_wq) { fen += 'Q'; }
		if (castle_bk) { fen += 'k'; }
		if (castle_bq) { fen += 'q'; }
	}

	fen += ' ';

	if (en_passant == -1) {
		fen += '-';
	} else {
		auto const file = en_passant % 8;
		auto const rank = en_passant / 8;

		fen += static_cast<char>('a' + file);
		fen += static_cast<char>('1' + rank);
	}

	fen += " " + std::to_string(halfmove_clock) + " 1";

	return fen;
}
} // namespace CastleMate