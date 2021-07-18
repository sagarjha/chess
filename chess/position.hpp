#pragma once

#include <string>
#include <vector>

#include "types.hpp"

class position {
public:
    position();
    position(const std::string& fen);
    position(const position& pos, const int start, const int end);
    position(const position& pos, const bool castle_side);

    std::vector<position> next_positions();

    void print() const;

private:
    int board[64];
    bool to_move;

    int enpassant_file;

    std::vector<int> piece_pos[6][2];

    std::array<int, 2> king_pos;
    std::array<std::array<bool, 2>, 2> castling_info;

    bool under_attack(const int square, bool whom);
};
