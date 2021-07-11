#pragma once

#include <vector>

#include "point.hpp"
#include "types.hpp"

bool get_color(char piece);

std::vector<point> get_moves(const board_t& board, const point cur);
