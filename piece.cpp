#include "piece.hpp"

bool get_color(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

std::vector<point> get_pawn_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves;

    if(color) {
        if(cur.x == 1) {
            if(!board[2][cur.y] && !board[3][cur.y]) {
                moves.push_back({3, cur.y});
            }
        }
        if(!board[cur.x + 1][cur.y]) {
            moves.push_back({cur.x + 1, cur.y});
        }
        if(cur.y > 0 && board[cur.x + 1][cur.y - 1] && !get_color(board[cur.x + 1][cur.y - 1])) {
            moves.push_back({cur.x + 1, cur.y - 1});
        }
        if(cur.y < 7 && board[cur.x + 1][cur.y + 1] && !get_color(board[cur.x + 1][cur.y + 1])) {
            moves.push_back({cur.x + 1, cur.y + 1});
        }
    } else {
        if(cur.x == 6) {
            if(!board[5][cur.y] && !board[4][cur.y]) {
                moves.push_back({4, cur.y});
            }
        }
        if(!board[cur.x - 1][cur.y]) {
            moves.push_back({cur.x - 1, cur.y});
        }
        if(cur.y > 0 && board[cur.x - 1][cur.y - 1] && get_color(board[cur.x - 1][cur.y - 1])) {
            moves.push_back({cur.x - 1, cur.y - 1});
        }
        if(cur.y < 7 && board[cur.x - 1][cur.y + 1] && !get_color(board[cur.x - 1][cur.y + 1])) {
            moves.push_back({cur.x - 1, cur.y + 1});
        }
    }
    return moves;
}

std::vector<point> get_knight_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves;
    const std::array<point, 8> dir = {{{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}}};
    for(const point p : dir) {
        point next = cur.add(p);
        if(next.valid() && (!board[next.x][next.y] || color != get_color(board[next.x][next.y]))) {
            moves.push_back(next);
        }
    }

    return moves;
}

std::vector<point> get_bishop_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves;
    const std::array<point, 4> dir = {{{1, 1}, {-1, 1}, {1, -1}, {-1, -1}}};
    for(const point p : dir) {
        point next = cur.add(p);
        while(next.valid() && !board[next.x][next.y]) {
            moves.push_back(next);
            next = next.add(p);
        }
        if(next.valid() && color != get_color(board[next.x][next.y])) {
            moves.push_back(next);
        }
    }

    return moves;
}

std::vector<point> get_rook_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves;
    const std::array<point, 4> dir = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    for(const point p : dir) {
        point next = cur.add(p);
        while(next.valid() && !board[next.x][next.y]) {
            moves.push_back(next);
            next = next.add(p);
        }
        if(next.valid() && color != get_color(board[next.x][next.y])) {
            moves.push_back(next);
        }
    }

    return moves;
}

std::vector<point> get_queen_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves = get_bishop_moves(board, cur, color);
    std::vector<point> rook_moves = get_rook_moves(board, cur, color);
    moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());

    return moves;
}

std::vector<point> get_king_moves(const board_t& board, const point cur, bool color) {
    std::vector<point> moves;
    const std::array<point, 8> dir = {{{1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}}};
    for(const point p : dir) {
        point next = cur.add(p);
        if(next.valid() && (!board[next.x][next.y] || color != get_color(board[next.x][next.y]))) {
            moves.push_back(next);
        }
    }

    return moves;
}

std::vector<point> get_moves(const board_t& board, const point cur, char piece) {
    bool color = get_color(piece);

    switch(piece) {
        case 'p':
        case 'P':
            return get_pawn_moves(board, cur, color);
            break;
        case 'n':
        case 'N':
            return get_knight_moves(board, cur, color);
            break;
        case 'b':
        case 'B':
            return get_bishop_moves(board, cur, color);
            break;
        case 'r':
        case 'R':
            return get_rook_moves(board, cur, color);
            break;
        case 'q':
        case 'Q':
            return get_queen_moves(board, cur, color);
            break;
        case 'k':
        case 'K':
            return get_king_moves(board, cur, color);
            break;
        default:
            throw "Invalid piece";
    }
}
