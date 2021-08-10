#include "piece.hpp"

int char_to_piece(const char ch) {
    switch(ch) {
        case 'p':
            return BLACK_PAWN;
        case 'n':
            return BLACK_KNIGHT;
        case 'b':
            return BLACK_BISHOP;
        case 'r':
            return BLACK_ROOK;
        case 'q':
            return BLACK_QUEEN;
        case 'k':
            return BLACK_KING;
        case 'P':
            return WHITE_PAWN;
        case 'N':
            return WHITE_KNIGHT;
        case 'B':
            return WHITE_BISHOP;
        case 'R':
            return WHITE_ROOK;
        case 'Q':
            return WHITE_QUEEN;
        case 'K':
            return WHITE_KING;
    }
    throw 0;
}

char piece_to_char(const int p) {
    switch(p/2) {
        case PAWN:
            return 'p' + !PCOLOR(p) * ('A' - 'a');
        case KNIGHT:
            return 'n' + !PCOLOR(p) * ('A' - 'a');
        case BISHOP:
            return 'b' + !PCOLOR(p) * ('A' - 'a');
        case ROOK:
            return 'r' + !PCOLOR(p) * ('A' - 'a');
        case QUEEN:
            return 'q' + !PCOLOR(p) * ('A' - 'a');
        case KING:
            return 'k' + !PCOLOR(p) * ('A' - 'a');
        case EMPTY/2:
	    return '.';
    }
    throw 0;
}
