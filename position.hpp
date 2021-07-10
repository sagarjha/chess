#pragma once

#include <vector>

#include "point.hpp"
#include "types.hpp"

class position {
public:
    std::vector<position> next_positions();

    void initialize();

    void print() const;

private:
    board_t board;
    bool side_to_move;
    point en_passant_square;

    position generate_position(const point start, const point end);
};
