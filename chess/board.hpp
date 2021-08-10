#pragma once

#include <map>
#include <vector>

#include "types.hpp"

class board_t {
public:
    void place(const int pos, const int piece);
    void remove(const int pos);
    void replace(const int src, const int dest);

    void find_checks(int enemy_pos[2], int side);
    int check_pinned(int pos);
    void find_king_moves(std::vector<move_t>& moves, int side);
    void find_king_moves(std::vector<move_t>& moves, int side, bool castling_info[2]);
    void find_non_king_moves(std::vector<move_t>& moves, int side, int pos);
    void find_captures(std::vector<move_t>& moves, int pos);
    void find_enpassant_captures(std::vector<move_t>& moves, int side, int enpassant_file);
    void find_blocking_moves(std::vector<move_t>& moves, int pos1, int side);
    void find_pawn_moves(std::vector<move_t>& moves, int pos, int enpassant_file);
    void find_pawn_moves(std::vector<move_t>& moves, int pos, int dir, int enpassant_file);
    void find_piece_moves(std::vector<move_t>& moves, int pos);
    void find_piece_moves(std::vector<move_t>& moves, int pos, int dir);
    void reset_pins_map();

    int& operator[](const std::size_t idx);
    const int& operator[](const std::size_t idx) const;

    board_t();

    int board[64];
    int king_pos[2];
    int knight_pos[2][10];
    std::map<int, int> pins;
};
