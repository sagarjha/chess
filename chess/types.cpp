#include "types.hpp"

const bool WHITE = false;
const bool BLACK = true;

const bool KINGSIDE = false;
const bool QUEENSIDE = true;

const int HOME_ROW[2] = {0, 7};
const int MIDDLE_ROW[2] = {3, 4};
const int PIECE_DIFF[2] = {'A' - 'a', 0};
const int PAWN_DIR[2] = {1, -1};

const char ALL_PIECES[6] = {'p', 'k', 'r', 'n', 'b', 'q'};
const char MAIN_PIECES[4] = {'r', 'n', 'b', 'q'};
const char EXCEPT_PAWNS[5] = {'r', 'n', 'b', 'q', 'k'};
