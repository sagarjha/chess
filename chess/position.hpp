#pragma once

#include <string>
#include <vector>

#include "board.hpp"
#include "types.hpp"

class position {
public:
    position();
    position(const std::string& fen);

    std::pair<std::pair<std::vector<move_t>, std::vector<move_t>>, memory_t> get_moves();

    void make_move(const move_t move);
    void make_king_move(const move_t move);
    void take_back(const move_t move, const memory_t m);
    void take_king_back(const move_t move, const memory_t m);

    void print() const;
    void print_fen() const;

private:
    board_t board;
    bool to_move;

    int enpassant_file;

    bool castling_info[2][2];

    bool under_attack(const int square, const bool whom);
};
