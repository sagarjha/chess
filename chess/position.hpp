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
    void print_trace() const;

private:
    board_t board;
    bool to_move;

    int enpassant_file;

    std::array<point, 2> king_pos;
    std::array<std::array<bool, 2>, 2> castling_info;

    std::string trace;

    bool under_attack(const point p, bool whom);

    position generate_position(const point start, const point end) const;
    position castle(bool side) const;
};
