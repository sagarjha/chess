#pragma once

#include <array>
#include <memory>
#include <optional>

#include "point.hpp"

typedef std::pair<point, point> move_t;
typedef std::array<std::array<char, 8>, 8> board_t;

extern const bool WHITE;
extern const bool BLACK;

extern const bool KINGSIDE;
extern const bool QUEENSIDE;

extern const int HOME_ROW[2];
extern const int MIDDLE_ROW[2];
extern const int PIECE_DIFF[2];
extern const int PAWN_DIR[2];

extern const char ALL_PIECES[6];
extern const char MAIN_PIECES[4];
extern const char EXCEPT_PAWNS[5];
