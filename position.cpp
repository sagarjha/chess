#include "position.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>

#include "piece.hpp"

position::position()
        : side_to_move(true),
          enpassant_file(-1),
          white_king_pos({0, 4}),
          black_king_pos({7, 4}),
          white_long_castle(true),
          white_short_castle(true),
          black_long_castle(true),
          black_short_castle(true) {
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }

    board[0][0] = 'R';
    board[0][1] = 'N';
    board[0][2] = 'B';
    board[0][3] = 'Q';
    board[0][4] = 'K';
    board[0][5] = 'B';
    board[0][6] = 'N';
    board[0][7] = 'R';

    board[7][0] = 'r';
    board[7][1] = 'n';
    board[7][2] = 'b';
    board[7][3] = 'q';
    board[7][4] = 'k';
    board[7][5] = 'b';
    board[7][6] = 'n';
    board[7][7] = 'r';

    for(int i = 0; i < 8; ++i) {
        board[1][i] = 'P';
        board[6][i] = 'p';
    }
}

position::position(const std::string& fen) {
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }

    point cur(7, 0);
    int fen_pos = 0;
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
                white_king_pos = cur;
            } else if(ch == 'k') {
                black_king_pos = cur;
            }
            --cur;
        }
    }

    fen_pos++;
    if(fen[fen_pos] == 'w') {
        side_to_move = true;
    } else {
        side_to_move = false;
    }
    fen_pos += 2;

    white_long_castle = false;
    white_short_castle = false;
    black_long_castle = false;
    black_short_castle = false;
    if(fen[fen_pos] == '-') {
        fen_pos++;
    } else {
        for(; fen[fen_pos] != ' '; ++fen_pos) {
            switch(fen[fen_pos]) {
                case 'K':
                    white_short_castle = true;
                    break;
                case 'Q':
                    white_long_castle = true;
                    break;
                case 'k':
                    black_short_castle = true;
                    break;
                case 'q':
                    black_long_castle = true;
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
            if(piece && (side_to_move == get_color(piece))) {
                std::vector<point> all_moves = get_moves(board, cur);
                for(const point move : all_moves) {
                    positions.push_back(generate_position(cur, move));
                    // promotion
                    if((piece == 'p' || piece == 'P') && (move.x == 0 || move.x == 7)) {
                        position pos = positions.back();
                        positions.pop_back();
                        pos.board[move.x][move.y] = 'n' + (int)side_to_move * ('A' - 'a');
                        positions.push_back(pos);
                        pos.board[move.x][move.y] = 'b' + (int)side_to_move * ('A' - 'a');
                        positions.push_back(pos);
                        pos.board[move.x][move.y] = 'r' + (int)side_to_move * ('A' - 'a');
                        positions.push_back(pos);
                        pos.board[move.x][move.y] = 'q' + (int)side_to_move * ('A' - 'a');
                        positions.push_back(pos);
                    }
                }
            }
            ++cur;
        }
    }

    // enpassant
    if(enpassant_file != -1) {
        if(side_to_move) {
            if(enpassant_file - 1 >= 0 && board[4][enpassant_file - 1] == 'P') {
                positions.push_back(generate_position({4, enpassant_file - 1}, {5, enpassant_file}));
                positions.back().board[4][enpassant_file] = 0;
                positions.back().trace += " ep ";
            }
            if(enpassant_file + 1 < 8 && board[4][enpassant_file + 1] == 'P') {
                positions.push_back(generate_position({4, enpassant_file + 1}, {5, enpassant_file}));
                positions.back().board[4][enpassant_file] = 0;
                positions.back().trace += " ep ";
            }
        } else {
            if(enpassant_file - 1 >= 0 && board[3][enpassant_file - 1] == 'p') {
                positions.push_back(generate_position({3, enpassant_file - 1}, {2, enpassant_file}));
                positions.back().board[3][enpassant_file] = 0;
                positions.back().trace += " ep ";
            }
            if(enpassant_file + 1 < 8 && board[3][enpassant_file + 1] == 'p') {
                positions.push_back(generate_position({3, enpassant_file + 1}, {2, enpassant_file}));
                positions.back().board[3][enpassant_file] = 0;
                positions.back().trace += " ep ";
            }
        }
    }

    // castling
    if(side_to_move) {
        if(white_long_castle) {
            if(!board[0][1] && !board[0][2] && !board[0][3]
               && !under_attack({0, 4}, 0) && !under_attack({0, 3}, 0) && !under_attack({0, 2}, 0)) {
                positions.push_back(castle(0));
            }
        }
        if(white_short_castle) {
            if(!board[0][5] && !board[0][6]
               && !under_attack({0, 4}, 0) && !under_attack({0, 5}, 0) && !under_attack({0, 6}, 0)) {
                positions.push_back(castle(1));
            }
        }
    } else {
        if(black_long_castle) {
            if(!board[7][1] && !board[7][2] && !board[7][3]
               && !under_attack({7, 4}, 1) && !under_attack({7, 3}, 1) && !under_attack({7, 2}, 1)) {
                positions.push_back(castle(0));
            }
        }
        if(black_short_castle) {
            if(!board[7][5] && !board[7][6]
               && !under_attack({7, 4}, 1) && !under_attack({7, 5}, 1) && !under_attack({7, 6}, 1)) {
                positions.push_back(castle(1));
            }
        }
    }

    // checks
    positions.erase(std::remove_if(positions.begin(), positions.end(), [](position& pos) {
                        if(pos.side_to_move) {
                            return pos.under_attack(pos.black_king_pos, 1);
                        }
                        return pos.under_attack(pos.white_king_pos, 0);
                    }),
                    positions.end());

    return positions;
}

bool position::under_attack(const point square, bool whom) {
    char original_piece = board[square.x][square.y];

    {
        board[square.x][square.y] = 'n' + (int)!whom * ('A' - 'a');
        std::vector<point> moves = get_moves(board, square);
        board[square.x][square.y] = original_piece;
        for(const point p : moves) {
            if(board[p.x][p.y] == 'n' || board[p.x][p.y] == 'N') {
                return true;
            }
        }
    }

    {
        board[square.x][square.y] = 'b' + (int)!whom * ('A' - 'a');
        std::vector<point> bishop_moves = get_moves(board, square);
        board[square.x][square.y] = original_piece;
        for(const point p : bishop_moves) {
            if(board[p.x][p.y] == 'b' || board[p.x][p.y] == 'B'
               || board[p.x][p.y] == 'q' || board[p.x][p.y] == 'Q') {
                return true;
            }
        }

        board[square.x][square.y] = 'r' + (int)!whom * ('A' - 'a');
        std::vector<point> rook_moves = get_moves(board, square);
        board[square.x][square.y] = original_piece;
        for(const point p : rook_moves) {
            if(board[p.x][p.y] == 'r' || board[p.x][p.y] == 'R'
               || board[p.x][p.y] == 'q' || board[p.x][p.y] == 'Q') {
                return true;
            }
        }
    }

    {
        board[square.x][square.y] = 'k' + (int)!whom * ('A' - 'a');
        std::vector<point> moves = get_moves(board, square, 0);
        board[square.x][square.y] = original_piece;
        for(const point p : moves) {
            char piece = board[p.x][p.y];
            if(board[p.x][p.y] == 'k' || board[p.x][p.y] == 'K') {
                return true;
            }
        }
    }

    {
        if(square.x != 0 + 7 * !whom) {
            board[square.x][square.y] = 'p' + (int)!whom * ('A' - 'a');
            std::vector<point> moves = get_moves(board, square);
            board[square.x][square.y] = original_piece;
            for(const point p : moves) {
                if(board[p.x][p.y] == 'p' || board[p.x][p.y] == 'P') {
                    return true;
                }
            }
        }
    }

    return false;
}

void position::print() const {
    std::cout << (side_to_move ? "W" : "B") << std::endl;
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

void position::print_trace() {
    std::cout << trace << " ";
}

position position::generate_position(const point start, const point end) {
    position new_pos(*this);

    const char piece = new_pos.board[start.x][start.y];
    new_pos.trace += piece;
    new_pos.trace += (end.y + 'a');
    new_pos.trace += (end.x + '1');
    new_pos.trace += ' ';

    if(new_pos.white_long_castle && end.x == 0 && end.y == 0) {
        new_pos.white_long_castle = false;
    } else if(new_pos.white_short_castle && end.x == 0 && end.y == 7) {
        new_pos.white_short_castle = false;
    } else if(new_pos.black_long_castle && end.x == 7 && end.y == 0) {
        new_pos.black_long_castle = false;
    } else if(new_pos.black_short_castle && end.x == 7 && end.y == 7) {
        new_pos.black_short_castle = false;
    }

    if(piece == 'k' || piece == 'K') {
        if(side_to_move) {
            new_pos.white_long_castle = false;
            new_pos.white_short_castle = false;
            new_pos.white_king_pos = end;
        } else {
            new_pos.black_long_castle = false;
            new_pos.black_short_castle = false;
            new_pos.black_king_pos = end;
        }
    }

    if(piece == 'r' || piece == 'R') {
        if(side_to_move) {
            if(start.x == 0 && start.y == 0) {
                new_pos.white_long_castle = false;
            }
            if(start.x == 0 && start.y == 7) {
                new_pos.white_short_castle = false;
            }
        } else {
            if(start.x == 7 && start.y == 0) {
                new_pos.black_long_castle = false;
            }
            if(start.x == 7 && start.y == 7) {
                new_pos.black_short_castle = false;
            }
        }
    }

    new_pos.board[end.x][end.y] = new_pos.board[start.x][start.y];
    new_pos.board[start.x][start.y] = 0;

    if((piece == 'p' || piece == 'P') && std::abs(end.x - start.x) == 2) {
        new_pos.enpassant_file = end.y;
    } else {
        new_pos.enpassant_file = -1;
    }

    new_pos.side_to_move = !side_to_move;

    return new_pos;
}

position position::castle(bool kingside) {
    position new_pos(*this);
    if(side_to_move) {
        new_pos.white_long_castle = false;
        new_pos.white_short_castle = false;
    } else {
        new_pos.black_long_castle = false;
        new_pos.black_short_castle = false;
    }

    if(side_to_move) {
        if(kingside) {
            new_pos.board[0][6] = 'K';
            new_pos.board[0][4] = 0;
            new_pos.white_king_pos = {0, 6};

            new_pos.board[0][5] = 'R';
            new_pos.board[0][7] = 0;

            new_pos.trace += "O-O";
        } else {
            new_pos.board[0][2] = 'K';
            new_pos.board[0][4] = 0;
            new_pos.white_king_pos = {0, 2};

            new_pos.board[0][3] = 'R';
            new_pos.board[0][0] = 0;

            new_pos.trace += "O-O-O";
        }
    } else {
        if(kingside) {
            new_pos.board[7][6] = 'k';
            new_pos.board[7][4] = 0;
            new_pos.black_king_pos = {7, 6};

            new_pos.board[7][5] = 'r';
            new_pos.board[7][7] = 0;

            new_pos.trace += "O-O";
        } else {
            new_pos.board[7][2] = 'k';
            new_pos.board[7][4] = 0;
            new_pos.black_king_pos = {7, 2};

            new_pos.board[7][3] = 'r';
            new_pos.board[7][0] = 0;

            new_pos.trace += "O-O-O";
        }
    }

    new_pos.enpassant_file = -1;

    new_pos.side_to_move = !side_to_move;

    return new_pos;
}
