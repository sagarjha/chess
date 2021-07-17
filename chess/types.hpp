/*
 * Some of the representation adapted from examples given at chessprogrammingwiki
*/

#pragma once

#include <array>
#include <memory>
#include <optional>

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

/* board representation */
#define WHITE 0
#define BLACK 1

#define WHITE_PAWN (2 * PAWN + WHITE)
#define BLACK_PAWN (2 * PAWN + BLACK)
#define WHITE_KNIGHT (2 * KNIGHT + WHITE)
#define BLACK_KNIGHT (2 * KNIGHT + BLACK)
#define WHITE_BISHOP (2 * BISHOP + WHITE)
#define BLACK_BISHOP (2 * BISHOP + BLACK)
#define WHITE_ROOK (2 * ROOK + WHITE)
#define BLACK_ROOK (2 * ROOK + BLACK)
#define WHITE_QUEEN (2 * QUEEN + WHITE)
#define BLACK_QUEEN (2 * QUEEN + BLACK)
#define WHITE_KING (2 * KING + WHITE)
#define BLACK_KING (2 * KING + BLACK)
#define EMPTY (BLACK_KING + 1)

#define PCOLOR(p) ((p)&1)

#define FLIP(sq) ((sq) ^ 56)

#define KINGSIDE 0
#define QUEENSIDE 1

#define HOME_ROW(side) (56 * !side)
#define MIDDLE_ROW(side) (24 + 8 * !side)
#define PAWN_DIR(side) (-8 + 16 * side)
