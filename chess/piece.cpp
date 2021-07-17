#include "piece.hpp"

#include <iostream>

std::vector<int> get_pawn_moves(const int board[64], const int cur, bool color);
std::vector<int> get_knight_moves(const int board[64], const int cur, bool color);
std::vector<int> get_bishop_moves(const int board[64], const int cur, bool color);
std::vector<int> get_rook_moves(const int board[64], const int cur, bool color);
std::vector<int> get_queen_moves(const int board[64], const int cur, bool color);
std::vector<int> get_king_moves(const int board[64], const int cur, bool color);

std::vector<int> knight_or_king_moves(const int board[64], const int cur, bool color,
const int dir[8]);
std::vector<int> bishop_or_rook_moves(const int board[64], const int cur, bool color,
const int dir[4]);

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

std::vector<int> get_moves(const int board[64], const int cur) {
    int piece = board[cur];
    bool color = PCOLOR(piece);

    switch(piece/2) {
        case PAWN:
            return get_pawn_moves(board, cur, color);
        case KNIGHT:
            return get_knight_moves(board, cur, color);
        case BISHOP:
            return get_bishop_moves(board, cur, color);
	case ROOK:
            return get_rook_moves(board, cur, color);
        case QUEEN:
            return get_queen_moves(board, cur, color);
        case KING:
            return get_king_moves(board, cur, color);
    }
    throw 0;
}

std::vector<int> get_pawn_moves(const int board[64], const int cur, bool color) {
    std::vector<int> moves;

    if(cur >= HOME_ROW(color) + PAWN_DIR(color) && cur < HOME_ROW(color) + PAWN_DIR(color) + 8) {
        if(board[cur + PAWN_DIR(color)] == EMPTY && board[cur + 2 * PAWN_DIR(color)] == EMPTY) {
            moves.push_back(cur + 2 * PAWN_DIR(color));
        }
    }
    if(board[cur + PAWN_DIR(color)] == EMPTY) {
        moves.push_back(cur + PAWN_DIR(color));
    }
    if(cur % 8 > 0 && board[cur + PAWN_DIR(color) - 1] != EMPTY
       && color != PCOLOR(board[cur + PAWN_DIR(color) - 1])) {
        moves.push_back(cur + PAWN_DIR(color) - 1);
    }
    if(cur % 8 < 7 && board[cur + PAWN_DIR(color) + 1] != EMPTY
       && color != PCOLOR(board[cur + PAWN_DIR(color) + 1])) {
        moves.push_back(cur + PAWN_DIR(color) + 1);
    }

    return moves;
}

std::vector<int> get_knight_moves(const int board[64], const int cur, bool color) {
    int dir[8] = {10, -6, 6, -10, 17, 15, -15, -17};
    return knight_or_king_moves(board, cur, color, dir);
}

std::vector<int> get_bishop_moves(const int board[64], const int cur, bool color) {
    int dir[4] = {9, -7, 7, -9};
    return bishop_or_rook_moves(board, cur, color, dir);
}

std::vector<int> get_rook_moves(const int board[64], const int cur, bool color) {
    int dir[4] = {8, -8, 1, -1};
    return bishop_or_rook_moves(board, cur, color, dir);
}

std::vector<int> get_queen_moves(const int board[64], const int cur, bool color) {
    std::vector<int> moves = get_bishop_moves(board, cur, color);
    std::vector<int> rook_moves = get_rook_moves(board, cur, color);
    moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());

    return moves;
}

std::vector<int> get_king_moves(const int board[64], const int cur, bool color) {
    int dir[8] = {7, 8, 9, 1, -7, -8, -9, -1};
    return knight_or_king_moves(board, cur, color, dir);
}

std::vector<int> knight_or_king_moves(const int board[64], const int cur, bool color,
const int dir[8]) {
    std::vector<int> moves;
    for(int i = 0; i < 8; ++i) {
        int next = cur + dir[i];
        if(next >= 0 && next < 64 && (board[next] == EMPTY || color != PCOLOR(board[next]))) {
            moves.push_back(next);
        }
    }
    
    return moves;
}

std::vector<int> bishop_or_rook_moves(const int board[64], const int cur, bool color,
const int dir[4]) {
    std::vector<int> moves;
    for(int i = 0; i < 4; ++i) {
        int next = cur + dir[i];
        while(next >= 0 && next < 64 && board[next] == EMPTY) {
            moves.push_back(next);
            next += dir[i];
        }
        if(next >= 0 && next < 64 && color != PCOLOR(board[next])) {
            moves.push_back(next);
        }
    }

    return moves;
}
