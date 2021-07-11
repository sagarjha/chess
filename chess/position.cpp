#include "position.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>

#include "piece.hpp"

position::position() : position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
}

position::position(const std::string& fen) {
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }

    point cur(7, 0);
    size_t fen_pos = 0;
    for(; fen_pos < fen.size() && cur.valid(); fen_pos++) {
        char ch = fen[fen_pos];
        if(ch == '/') {
            continue;
        }
        if(std::isdigit(ch)) {
            for(int j = 0; j < ch - '0'; ++j) {
                --cur;
            }
        } else {
            board[cur.x][cur.y] = ch;
            if(ch == 'K') {
                king_pos[WHITE] = cur;
            } else if(ch == 'k') {
                king_pos[BLACK] = cur;
            }
            --cur;
        }
    }

    fen_pos++;
    if(fen[fen_pos] == 'w') {
        to_move = WHITE;
    } else {
        to_move = BLACK;
    }
    fen_pos += 2;

    castling_info[WHITE] = {false, false};
    castling_info[BLACK] = {false, false};
    if(fen[fen_pos] == '-') {
        fen_pos++;
    } else {
        for(; fen[fen_pos] != ' '; ++fen_pos) {
            switch(fen[fen_pos]) {
                case 'K':
                    castling_info[WHITE][KINGSIDE] = true;
                    break;
                case 'Q':
                    castling_info[WHITE][QUEENSIDE] = true;
                    break;
                case 'k':
                    castling_info[BLACK][KINGSIDE] = true;
                    break;
                case 'q':
                    castling_info[BLACK][QUEENSIDE] = true;
                    break;
            }
        }
    }
    fen_pos++;

    if(fen[fen_pos] == '-') {
        enpassant_file = -1;
        fen_pos++;
    } else {
        enpassant_file = fen[fen_pos] - 'a';
        fen_pos += 2;
    }
}

std::vector<position> position::next_positions() {
    std::vector<position> positions;

    point cur;
    for(const std::array<char, 8>& ranks : board) {
        for(const char piece : ranks) {
            if(piece && (to_move == get_color(piece))) {
                std::vector<point> all_moves = get_moves(board, cur);
                for(const point move : all_moves) {
                    positions.push_back(generate_position(cur, move));

                    // promotion
                    if((piece == 'p' || piece == 'P') && (move.x == 0 || move.x == 7)) {
                        position pos = positions.back();
                        positions.pop_back();
                        for(char piece : MAIN_PIECES) {
                            pos.board[move.x][move.y] = piece + PIECE_DIFF[to_move];
                            positions.push_back(pos);
                        }
                    }
                }
            }
            ++cur;
        }
    }

    // enpassant
    if(enpassant_file != -1) {
        if(enpassant_file - 1 >= 0 && board[MIDDLE_ROW[!to_move]][enpassant_file - 1] == 'p' + PIECE_DIFF[to_move]) {
            positions.push_back(generate_position({MIDDLE_ROW[!to_move], enpassant_file - 1},
                                                  {MIDDLE_ROW[!to_move] + PAWN_DIR[to_move], enpassant_file}));
            positions.back().board[MIDDLE_ROW[!to_move]][enpassant_file] = 0;
            positions.back().trace += " ep ";
        }
        if(enpassant_file + 1 < 8 && board[MIDDLE_ROW[!to_move]][enpassant_file + 1] == 'p' + PIECE_DIFF[to_move]) {
            positions.push_back(generate_position({MIDDLE_ROW[!to_move], enpassant_file + 1},
                                                  {MIDDLE_ROW[!to_move] + PAWN_DIR[to_move], enpassant_file}));
            positions.back().board[MIDDLE_ROW[!to_move]][enpassant_file] = 0;
            positions.back().trace += " ep ";
        }
    }

    // castling
    if(castling_info[to_move][QUEENSIDE]
       && !board[HOME_ROW[to_move]][1] && !board[HOME_ROW[to_move]][2] && !board[HOME_ROW[to_move]][3]
       && !under_attack({HOME_ROW[to_move], 2}, !to_move) && !under_attack({HOME_ROW[to_move], 3}, !to_move)
       && !under_attack({HOME_ROW[to_move], 4}, !to_move)) {
        positions.push_back(castle(QUEENSIDE));
    }
    if(castling_info[to_move][KINGSIDE]
       && !board[HOME_ROW[to_move]][5] && !board[HOME_ROW[to_move]][6]
       && !under_attack({HOME_ROW[to_move], 4}, !to_move) && !under_attack({HOME_ROW[to_move], 5}, !to_move)
       && !under_attack({HOME_ROW[to_move], 6}, !to_move)) {
        positions.push_back(castle(KINGSIDE));
    }

    // checks
    positions.erase(std::remove_if(positions.begin(), positions.end(), [](position& pos) {
                        return pos.under_attack(pos.king_pos[!pos.to_move], pos.to_move);
                    }),
                    positions.end());

    return positions;
}

bool position::under_attack(const point square, bool whom) {
    char original_piece = board[square.x][square.y];

    std::function<bool(char)> test = [this, original_piece, square, whom](char piece) {
        board[square.x][square.y] = piece + PIECE_DIFF[!whom];
        std::vector<point> moves = get_moves(board, square);
        board[square.x][square.y] = original_piece;
        for(const point p : moves) {
            if(board[p.x][p.y] == piece || board[p.x][p.y] == piece + 'A' - 'a') {
                return true;
            }
        }
        return false;
    };

    for(char piece : EXCEPT_PAWNS) {
        if(test(piece)) {
            return true;
        }
    }
    if(square.x != HOME_ROW[whom] && test('p')) {
        return true;
    }

    return false;
}

void position::print() const {
    std::cout << (to_move == WHITE ? "W" : "B") << std::endl;
    for(int x = 7; x >= 0; --x) {
        for(int y = 0; y < 8; ++y) {
            if(board[x][y]) {
                std::cout << board[x][y];
            } else {
                std::cout << '.';
            }
        }
        std::cout << std::endl;
    }
    std::cout << trace << std::endl;
    std::cout << std::endl;
}

void position::print_trace() const {
    std::cout << trace << " ";
}

position position::generate_position(const point start, const point end) const {
    position new_pos(*this);

    const char piece = new_pos.board[start.x][start.y];
    new_pos.trace += piece;
    new_pos.trace += (end.y + 'a');
    new_pos.trace += (end.x + '1');
    new_pos.trace += ' ';

    new_pos.castling_info[WHITE][QUEENSIDE] &= !(end.x == 0 && end.y == 0);
    new_pos.castling_info[WHITE][KINGSIDE] &= !(end.x == 0 && end.y == 7);
    new_pos.castling_info[BLACK][QUEENSIDE] &= !(end.x == 7 && end.y == 0);
    new_pos.castling_info[BLACK][KINGSIDE] &= !(end.x == 7 && end.y == 7);

    if(piece == 'k' || piece == 'K') {
        new_pos.castling_info[to_move] = {false, false};
	new_pos.king_pos[to_move] = end;
    }

    if(piece == 'r' || piece == 'R') {
        new_pos.castling_info[to_move][QUEENSIDE] &= !(start.x == HOME_ROW[to_move] && start.y == 0);
        new_pos.castling_info[to_move][KINGSIDE] &= !(start.x == HOME_ROW[to_move] && start.y == 7);
    }

    new_pos.board[end.x][end.y] = new_pos.board[start.x][start.y];
    new_pos.board[start.x][start.y] = 0;

    if((piece == 'p' || piece == 'P') && std::abs(end.x - start.x) == 2) {
        new_pos.enpassant_file = end.y;
    } else {
        new_pos.enpassant_file = -1;
    }

    new_pos.to_move = !to_move;

    return new_pos;
}

position position::castle(bool side) const {
    position new_pos(*this);

    new_pos.castling_info[to_move][QUEENSIDE] = false;
    new_pos.castling_info[to_move][KINGSIDE] = false;

    if(side == QUEENSIDE) {
        new_pos.board[HOME_ROW[to_move]][2] = 'k' + PIECE_DIFF[to_move];
        new_pos.board[HOME_ROW[to_move]][4] = 0;
        new_pos.king_pos[to_move] = {HOME_ROW[to_move], 2};

        new_pos.board[HOME_ROW[to_move]][3] = 'r' + PIECE_DIFF[to_move];
        new_pos.board[HOME_ROW[to_move]][0] = 0;

        new_pos.trace += "O-O-O";
    } else {
        new_pos.board[HOME_ROW[to_move]][6] = 'k' + PIECE_DIFF[to_move];
        new_pos.board[HOME_ROW[to_move]][4] = 0;
        new_pos.king_pos[to_move] = {HOME_ROW[to_move], 6};

        new_pos.board[HOME_ROW[to_move]][5] = 'r' + PIECE_DIFF[to_move];
        new_pos.board[HOME_ROW[to_move]][7] = 0;

        new_pos.trace += "O-O";
    }

    new_pos.enpassant_file = -1;
    new_pos.to_move = !to_move;

    return new_pos;
}
