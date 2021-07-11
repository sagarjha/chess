#pragma once

#include <string>
#include <vector>

#include "point.hpp"
#include "types.hpp"

class position {
public:
    position();
    position(const std::string& fen);

    std::vector<position> next_positions();

    void print() const;
    void print_trace();

private:
    board_t board;
    bool side_to_move;
    
    int enpassant_file;

    point white_king_pos;
    point black_king_pos;
    bool white_long_castle;
    bool white_short_castle;
    bool black_long_castle;
    bool black_short_castle;

    std::string trace;

    bool under_attack(const point p, bool whom);

    position generate_position(const point start, const point end);
    position castle(bool kingside);
};
