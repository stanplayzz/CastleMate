#include "castlemate/core/position.hpp"
#include "castlemate/utils/bit_math.hpp"
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
	stream >> board >> turn >> castling >> en_passant;

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

	return pos;
}
} // namespace CastleMate