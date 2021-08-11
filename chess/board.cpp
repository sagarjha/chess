#include "board.hpp"

#include <algorithm>

#include "piece.hpp"

std::vector<std::array<int, 8>> knight_jumps;
std::function<int(int board[64], int)> find_next[8];
int slider_dir[8] = {1, -1, 8, -8, 7, -7, 9, -9};
bool is_initialized = false;
void initialize();
int find_alignment(int pos1, int pos2);
void normal_move(int board[64], std::vector<move_t>& moves, int cur, int dest);
bool under_attack(int board[64], int pos, int side);
void bishop_or_rook_moves(int board[64], std::vector<move_t>& moves,
                          int cur, const int dir[4]);
std::array<int, 8>& get_knight_jumps(int pos);

void board_t::place(const int pos, const int piece) {
  board[pos] = piece;
}

void board_t::place_king(const int pos, const int piece) {
    board[pos] = piece;
    king_pos[PCOLOR(piece)] = pos;
}

void board_t::remove(const int pos) {
    board[pos] = EMPTY;
}

void board_t::replace(const int src, const int dest) {
    board[dest] = board[src];
    board[src] = EMPTY;
}

void board_t::replace_king(const int src, const int dest) {
    board[dest] = board[src];
    board[src] = EMPTY;
    king_pos[PCOLOR(board[dest])] = dest;
}

void board_t::find_checks(int enemy_pos[2], int side) {
    enemy_pos[0] = enemy_pos[1] = -1;
    int num_checks = 0;

    for(int dir = 0; dir < 8; ++dir) {
        int next = find_next[dir](board, king_pos[side]);
        if(next != -1) {
            int piece = board[next];
            if((piece / 2 == QUEEN || piece / 2 == SLIDER(dir)) && PCOLOR(piece) != side) {
                enemy_pos[num_checks++] = next;
            } else if(dir >= 4 && next - king_pos[side] == slider_dir[dir] && piece == 2 * PAWN + !side
                      && slider_dir[dir] * PAWN_DIR(!side) < 0) {
                enemy_pos[num_checks++] = next;
            }
        }
    }

    for(int dest : get_knight_jumps(king_pos[side])) {
        if(board[dest] == 2 * KNIGHT + !side) {
            enemy_pos[num_checks++] = dest;
            break;
        }
    }
}

int board_t::check_pinned(int pos) {
    if(pins[pos] != -2) {
        return pins[pos];
    }

    int side = PCOLOR(board[pos]);

    int dir = find_alignment(king_pos[side], pos);
    if(dir == -1) {
        pins[pos] = -1;
        return -1;
    }

    int pos1 = find_next[2 * dir](board, pos);
    int pos2 = find_next[2 * dir + 1](board, pos);
    if(pos1 == -1 || pos2 == -1) {
        pins[pos] = -1;
        return -1;
    }

    int piece1 = board[pos1];
    int piece2 = board[pos2];

    if(piece1 == 2 * KING + side) {
        if((piece2 / 2 == QUEEN || piece2 / 2 == SLIDER(2 * dir)) && PCOLOR(piece2) != side) {
            pins[pos] = dir;
            return dir;
        }
    } else if(piece2 == 2 * KING + side) {
        if((piece1 / 2 == QUEEN || piece1 / 2 == SLIDER(2 * dir)) && PCOLOR(piece1) != side) {
            pins[pos] = dir;
            return dir;
        }
    }

    pins[pos] = -1;
    return -1;
}

void board_t::find_king_moves(std::vector<move_t>& moves, int side) {
    remove(king_pos[side]);
    for(int dir : {7, 8, 9, 1, -7, -8, -9, -1}) {
        int next = king_pos[side] + dir;
        if(next >= 0 && next < 64 && std::abs((next % 8) - (king_pos[side] % 8)) <= 1
           && (board[next] == EMPTY || side != PCOLOR(board[next]))
           && !under_attack(board, next, !side)) {
            place(king_pos[side], 2 * KING + side);
            normal_move(board, moves, king_pos[side], next);
            remove(king_pos[side]);
        }
    }
    place(king_pos[side], 2 * KING + side);
}

void board_t::find_king_moves(std::vector<move_t>& moves, int side, bool castling_info[2]) {
    find_king_moves(moves, side);

    if(castling_info[QUEENSIDE]
       && board[HOME_ROW(side) + 1] == EMPTY && board[HOME_ROW(side) + 2] == EMPTY
       && board[HOME_ROW(side) + 3] == EMPTY
       && !under_attack(board, HOME_ROW(side) + 2, !side) && !under_attack(board, HOME_ROW(side) + 3, !side)
       && !under_attack(board, HOME_ROW(side) + 4, !side)) {
        moves.push_back({{2 * KING + side, {HOME_ROW(side) + 4, HOME_ROW(side) + 2}},
                         {2 * ROOK + side, {HOME_ROW(side), HOME_ROW(side) + 3}},
                         {-1, {-1, -1}}});
    }
    if(castling_info[KINGSIDE]
       && board[HOME_ROW(side) + 5] == EMPTY && board[HOME_ROW(side) + 6] == EMPTY
       && !under_attack(board, HOME_ROW(side) + 4, !side) && !under_attack(board, HOME_ROW(side) + 5, !side)
       && !under_attack(board, HOME_ROW(side) + 6, !side)) {
        moves.push_back({{2 * KING + side, {HOME_ROW(side) + 4, HOME_ROW(side) + 6}},
                         {2 * ROOK + side, {HOME_ROW(side) + 7, HOME_ROW(side) + 5}},
                         {-1, {-1, -1}}});
    }
}
void board_t::find_captures(std::vector<move_t>& moves, int pos) {
    find_non_king_moves(moves, !PCOLOR(board[pos]), pos);
}

void board_t::find_enpassant_captures(std::vector<move_t>& moves, int side, int enpassant_file) {
    if(enpassant_file > 0) {
        int pos = MIDDLE_ROW(!side) + enpassant_file - 1;
        if(board[pos] == 2 * PAWN + side && check_pinned(pos) == -1) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file - 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    }
    if(enpassant_file < 7) {
        int pos = MIDDLE_ROW(!side) + enpassant_file + 1;
        if(board[pos] == 2 * PAWN + side && check_pinned(pos) == -1) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file + 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    }
}

void board_t::find_blocking_moves(std::vector<move_t>& moves, int pos1, int side) {
    int dir = find_alignment(pos1, king_pos[side]);
    dir = slider_dir[2 * dir];
    for(int pos = std::min(pos1, king_pos[side]) + dir; pos < std::max(pos1, king_pos[side]); pos += dir) {
        find_non_king_moves(moves, side, pos);
    }
}

void board_t::find_pawn_moves(std::vector<move_t>& moves, int pos, int enpassant_file) {
    int side = PCOLOR(board[pos]);
    if(pos >= HOME_ROW(side) + PAWN_DIR(side) && pos < HOME_ROW(side) + PAWN_DIR(side) + 8) {
        if(board[pos + PAWN_DIR(side)] == EMPTY && board[pos + 2 * PAWN_DIR(side)] == EMPTY) {
            normal_move(board, moves, pos, pos + 2 * PAWN_DIR(side));
        }
    }
    if(board[pos + PAWN_DIR(side)] == EMPTY) {
        int next = pos + PAWN_DIR(side);
        if(next < 8 || next >= 56) {
            for(int promoted_piece = KNIGHT; promoted_piece <= QUEEN; ++promoted_piece) {
                moves.push_back({{board[pos], {pos, GRAVE}},
                                 {-1, {-1, -1}},
                                 {2 * promoted_piece + side, {GRAVE, next}}});
            }
        } else {
            normal_move(board, moves, pos, next);
        }
    }
    if(pos % 8 > 0 && board[pos + PAWN_DIR(side) - 1] != EMPTY
       && side != PCOLOR(board[pos + PAWN_DIR(side) - 1])) {
        int next = pos + PAWN_DIR(side) - 1;
        if(next < 8 || next >= 56) {
            for(int promoted_piece = KNIGHT; promoted_piece <= QUEEN; ++promoted_piece) {
                moves.push_back({{board[pos], {pos, GRAVE}},
                                 {board[next], {next, GRAVE}},
                                 {2 * promoted_piece + side, {GRAVE, next}}});
            }
        } else {
            normal_move(board, moves, pos, next);
        }
    }
    if(pos % 8 < 7 && board[pos + PAWN_DIR(side) + 1] != EMPTY
       && side != PCOLOR(board[pos + PAWN_DIR(side) + 1])) {
        int next = pos + PAWN_DIR(side) + 1;
        if(next < 8 || next >= 56) {
            for(int promoted_piece = KNIGHT; promoted_piece <= QUEEN; ++promoted_piece) {
                moves.push_back({{board[pos], {pos, GRAVE}},
                                 {board[next], {next, GRAVE}},
                                 {2 * promoted_piece + side, {GRAVE, next}}});
            }
        } else {
            normal_move(board, moves, pos, next);
        }
    }

    if(enpassant_file == -1) {
        return;
    }
    if(enpassant_file > 0 && pos == MIDDLE_ROW(!side) + enpassant_file - 1) {
        remove(MIDDLE_ROW(!side) + enpassant_file);
        bool possible = true;
        int dir = find_alignment(king_pos[side], pos);
        if(dir == 0) {
            int pos1 = find_next[0](board, pos);
            int pos2 = find_next[1](board, pos);
            if(pos1 != -1 && pos2 != -1) {
                int piece1 = board[pos1];
                int piece2 = board[pos2];
                if(piece1 == 2 * KING + side) {
                    if((piece2 / 2 == QUEEN || piece2 / 2 == ROOK) && PCOLOR(piece2) != side) {
                        possible = false;
                    }
                } else if(piece2 == 2 * KING + side) {
                    if((piece1 / 2 == QUEEN || piece1 / 2 == ROOK) && PCOLOR(piece1) != side) {
                        possible = false;
                    }
                }
            }
        }
        place(MIDDLE_ROW(!side) + enpassant_file, 2 * PAWN + !side);
        if(possible) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file - 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    }
    if(enpassant_file < 7 && pos == MIDDLE_ROW(!side) + enpassant_file + 1) {
        remove(MIDDLE_ROW(!side) + enpassant_file);
        bool possible = true;
        int dir = find_alignment(king_pos[side], pos);
        if(dir == 0) {
            int pos1 = find_next[0](board, pos);
            int pos2 = find_next[1](board, pos);
            if(pos1 != -1 && pos2 != -1) {
                int piece1 = board[pos1];
                int piece2 = board[pos2];
                if(piece1 == 2 * KING + side) {
                    if((piece2 / 2 == QUEEN || piece2 / 2 == ROOK) && PCOLOR(piece2) != side) {
                        possible = false;
                    }
                } else if(piece2 == 2 * KING + side) {
                    if((piece1 / 2 == QUEEN || piece1 / 2 == ROOK) && PCOLOR(piece1) != side) {
                        possible = false;
                    }
                }
            }
        }
        place(MIDDLE_ROW(!side) + enpassant_file, 2 * PAWN + !side);
        if(possible) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file + 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    }
}

void board_t::find_pawn_moves(std::vector<move_t>& moves, int pos, int dir, int enpassant_file) {
    int side = PCOLOR(board[pos]);
    if(dir == 1) {
        if(pos >= HOME_ROW(side) + PAWN_DIR(side) && pos < HOME_ROW(side) + PAWN_DIR(side) + 8) {
            if(board[pos + PAWN_DIR(side)] == EMPTY && board[pos + 2 * PAWN_DIR(side)] == EMPTY) {
                normal_move(board, moves, pos, pos + 2 * PAWN_DIR(side));
            }
        }
        if(board[pos + PAWN_DIR(side)] == EMPTY) {
            int next = pos + PAWN_DIR(side);
            normal_move(board, moves, pos, next);
        }
    } else if((dir == 2 && side == BLACK) || (dir == 3 && side == WHITE)) {
        if(pos % 8 > 0 && board[pos + PAWN_DIR(side) - 1] != EMPTY
           && side != PCOLOR(board[pos + PAWN_DIR(side) - 1])) {
            int next = pos + PAWN_DIR(side) - 1;
            normal_move(board, moves, pos, next);
        }
        if(enpassant_file == -1) {
            return;
        }
        if(enpassant_file < 7 && pos == MIDDLE_ROW(!side) + enpassant_file + 1) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file + 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    } else if((dir == 3 && side == BLACK) || (dir == 2 && side == WHITE)) {
        if(pos % 8 < 7 && board[pos + PAWN_DIR(side) + 1] != EMPTY
           && side != PCOLOR(board[pos + PAWN_DIR(side) + 1])) {
            int next = pos + PAWN_DIR(side) + 1;
            normal_move(board, moves, pos, next);
        }
        if(enpassant_file == -1) {
            return;
        }
        if(enpassant_file > 0 && pos == MIDDLE_ROW(!side) + enpassant_file - 1) {
            moves.push_back({{2 * PAWN + side,
                              {MIDDLE_ROW(!side) + enpassant_file - 1,
                               MIDDLE_ROW(!side) + PAWN_DIR(side) + enpassant_file}},
                             {2 * PAWN + !side, {MIDDLE_ROW(!side) + enpassant_file, GRAVE}},
                             {-1, {-1, -1}}});
        }
    }
}

void board_t::find_piece_moves(std::vector<move_t>& moves, int pos) {
    int piece = board[pos];
    int side = PCOLOR(piece);

    int bishop_dir[4] = {9, -7, 7, -9};
    int rook_dir[4] = {8, -8, 1, -1};

    if(piece / 2 == KNIGHT) {
        for(int dir : {10, -6, 6, -10, 17, 15, -15, -17}) {
            int next = pos + dir;
            if(next >= 0 && next < 64 && std::abs((next % 8) - (pos % 8)) <= 2
               && (board[next] == EMPTY || side != PCOLOR(board[next]))) {
                normal_move(board, moves, pos, next);
            }
        }
    } else if(piece / 2 == BISHOP) {
        bishop_or_rook_moves(board, moves, pos, bishop_dir);
    } else if(piece / 2 == ROOK) {
        bishop_or_rook_moves(board, moves, pos, rook_dir);
    } else if(piece / 2 == QUEEN) {
        bishop_or_rook_moves(board, moves, pos, bishop_dir);
        bishop_or_rook_moves(board, moves, pos, rook_dir);
    }
}

void board_t::find_piece_moves(std::vector<move_t>& moves, int pos, int dir) {
    int piece = board[pos];
    int side = PCOLOR(piece);

    std::function<void(int dir)> move_in_dir = [&](int dir) {
        int next = pos + dir;
        int prev = pos;
        while(next >= 0 && next < 64 && std::abs(next % 8 - prev % 8) <= 1
              && board[next] == EMPTY) {
            normal_move(board, moves, pos, next);
            next += dir;
            prev += dir;
        }
        if(next >= 0 && next < 64 && std::abs(next % 8 - prev % 8) <= 1
           && side != PCOLOR(board[next])) {
            normal_move(board, moves, pos, next);
        }
    };

    if(piece / 2 == SLIDER(2 * dir) || piece / 2 == QUEEN) {
        move_in_dir(slider_dir[2 * dir]);
        move_in_dir(slider_dir[2 * dir + 1]);
    }
}

void board_t::reset_pins() {
    for(int i = 0; i < 64; ++i) {
        pins[i] = -2;
    }
}

int& board_t::operator[](const std::size_t idx) {
    return board[idx];
}

const int& board_t::operator[](const std::size_t idx) const {
    return board[idx];
}

board_t::board_t() {
    initialize();
    for(int i = 0; i < 64; ++i) {
        board[i] = EMPTY;
    }
    reset_pins();
}

void initialize() {
    if(is_initialized) {
        return;
    }

    knight_jumps.resize(64, {-1, -1, -1, -1, -1, -1, -1, -1});

    std::function<int(int[], int, int, int)> next_in_range =
            [](int board[64], int first, int last, int step) {
                if(step > 0) {
                    for(int cur = first + step; cur <= last; cur += step) {
                        if(board[cur] != EMPTY) {
                            return cur;
                        }
                    }
                } else {
                    for(int cur = first + step; cur >= last; cur += step) {
                        if(board[cur] != EMPTY) {
                            return cur;
                        }
                    }
                }

                return -1;
            };

    find_next[0] = [next_in_range](int board[64], int pos) {
        int last = pos + 7 - (pos % 8);
        return next_in_range(board, pos, last, 1);
    };

    find_next[1] = [next_in_range](int board[64], int pos) {
        int last = pos - (pos % 8);
        return next_in_range(board, pos, last, -1);
    };

    find_next[2] = [next_in_range](int board[64], int pos) {
        int last = 56 + (pos % 8);
        return next_in_range(board, pos, last, 8);
    };

    find_next[3] = [next_in_range](int board[64], int pos) {
        int last = pos % 8;
        return next_in_range(board, pos, last, -8);
    };

    find_next[4] = [next_in_range](int board[64], int pos) {
        int last = pos + 7 * std::min(7 - pos / 8, pos % 8);
        return next_in_range(board, pos, last, 7);
    };

    find_next[5] = [next_in_range](int board[64], int pos) {
        int last = pos - 7 * std::min(pos / 8, 7 - pos % 8);
        return next_in_range(board, pos, last, -7);
    };

    find_next[6] = [next_in_range](int board[64], int pos) {
        int last = pos + 9 * std::min(7 - pos / 8, 7 - pos % 8);
        return next_in_range(board, pos, last, 9);
    };

    find_next[7] = [next_in_range](int board[64], int pos) {
        int last = pos - 9 * std::min(pos / 8, pos % 8);
        return next_in_range(board, pos, last, -9);
    };

    is_initialized = true;
}

int find_alignment(int pos1, int pos2) {
    if(pos2 - pos1 < 8) {
        int first = pos1 - (pos1 % 8);
        int last = pos1 + 7 - (pos1 % 8);
        if(pos2 >= first && pos2 <= last) {
            return 0;
        }
    }

    if((pos2 - pos1) % 8 == 0) {
        int first = pos1 % 8;
        int last = 56 + (pos1 % 8);
        if(pos2 >= first && pos2 <= last) {
            return 1;
        }
    }

    if((pos2 - pos1) % 7 == 0) {
        int first = pos1 - 7 * std::min(pos1 / 8, 7 - pos1 % 8);
        int last = pos1 + 7 * std::min(7 - pos1 / 8, pos1 % 8);
        if(pos2 >= first && pos2 <= last) {
            return 2;
        }
    }

    if((pos2 - pos1) % 9 == 0) {
        int first = pos1 - 9 * std::min(pos1 / 8, pos1 % 8);
        int last = pos1 + 9 * std::min(7 - pos1 / 8, 7 - pos1 % 8);
        if(pos2 >= first && pos2 <= last) {
            return 3;
        }
    }

    return -1;
}

void normal_move(int board[64], std::vector<move_t>& moves, int cur, int dest) {
    int piece = board[cur];
    if(board[dest] == EMPTY) {
        moves.push_back({{piece, {cur, dest}}, {-1, {-1, -1}}, {-1, {-1, -1}}});
    } else {
        moves.push_back({{piece, {cur, dest}}, {board[dest], {dest, GRAVE}}, {-1, {-1, -1}}});
    }
};

std::array<int, 8>& get_knight_jumps(int pos) {
    if(knight_jumps[pos][0] != -1) {
        return knight_jumps[pos];
    }
    int i = 0;
    for(int dir : {10, -6, 6, -10, 17, 15, -15, -17}) {
        int next = pos + dir;
        if(next >= 0 && next < 64 && std::abs((next % 8) - (pos % 8)) <= 2) {
            knight_jumps[pos][i++] = next;
        }
    }
    return knight_jumps[pos];
}

void board_t::find_non_king_moves(std::vector<move_t>& moves, int side, int pos) {
    for(int dir = 0; dir < 8; ++dir) {
        int next = find_next[dir](board, pos);
        if(next != -1) {
            int piece = board[next];
            if((piece / 2 == QUEEN || piece / 2 == SLIDER(dir)) && PCOLOR(piece) == side
               && (check_pinned(next) == -1 || check_pinned(next) == dir / 2)) {
                normal_move(board, moves, next, pos);
            } else if((next - pos == slider_dir[dir] || next - pos == 2 * slider_dir[dir])
                      && dir >= 2 && piece == 2 * PAWN + side
                      && slider_dir[dir] * PAWN_DIR(side) < 0
                      && (check_pinned(next) == -1 || check_pinned(next) == dir / 2)) {
                if(next - pos == slider_dir[dir]
                   && ((dir < 4 && board[pos] == EMPTY) || (dir >= 4 && board[pos] != EMPTY))) {
                    if(pos < 8 || pos >= 56) {
                        for(int promoted_piece = KNIGHT; promoted_piece <= QUEEN; ++promoted_piece) {
                            if(board[next] == EMPTY) {
                                moves.push_back({{piece, {next, GRAVE}},
                                                 {-1, {-1, -1}},
                                                 {2 * promoted_piece + side, {GRAVE, pos}}});
                            } else {
                                moves.push_back({{piece, {next, GRAVE}},
                                                 {board[pos], {pos, GRAVE}},
                                                 {2 * promoted_piece + side, {GRAVE, pos}}});
                            }
                        }
                    } else {
                        normal_move(board, moves, next, pos);
                    }
                } else if(next - pos == 2 * slider_dir[dir] && dir < 4
                          && board[pos] == EMPTY && next >= HOME_ROW(side) + PAWN_DIR(side)
                          && next < HOME_ROW(side) + PAWN_DIR(side) + 8) {
                    normal_move(board, moves, next, pos);
                }
            }
        }
    }

    for(int dest : get_knight_jumps(pos)) {
        if(board[dest] == 2 * KNIGHT + side && check_pinned(dest) == -1) {
            normal_move(board, moves, dest, pos);
        }
    }
}

bool under_attack(int board[64], int pos, int side) {
    for(int dir = 0; dir < 8; ++dir) {
        int next = find_next[dir](board, pos);
        if(next != -1) {
            int piece = board[next];
            if((piece / 2 == QUEEN || piece / 2 == SLIDER(dir)) && PCOLOR(piece) == side) {
                return true;
            } else if(next - pos == slider_dir[dir]) {
                if(dir >= 4 && piece == 2 * PAWN + side
                   && slider_dir[dir] * PAWN_DIR(side) < 0) {
                    return true;
                } else if(piece == 2 * KING + side) {
                    return true;
                }
            }
        }
    }

    for(int dest : get_knight_jumps(pos)) {
        if(board[dest] == 2 * KNIGHT + side) {
            return true;
        }
    }

    return false;
}

void bishop_or_rook_moves(int board[64], std::vector<move_t>& moves,
                          int cur, const int dir[4]) {
    int side = PCOLOR(board[cur]);
    for(int i = 0; i < 4; ++i) {
        int next = cur + dir[i];
        int prev = cur;
        while(next >= 0 && next < 64 && std::abs(next % 8 - prev % 8) <= 1
              && board[next] == EMPTY) {
            normal_move(board, moves, cur, next);
            next += dir[i];
            prev += dir[i];
        }
        if(next >= 0 && next < 64 && std::abs(next % 8 - prev % 8) <= 1
           && side != PCOLOR(board[next])) {
            normal_move(board, moves, cur, next);
        }
    }
}
