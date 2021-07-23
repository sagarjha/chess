#pragma once

#include <vector>

#include "types.hpp"

int char_to_piece(const char ch);
char piece_to_char(const int p);

std::vector<int> get_piece_moves(const int board[64], const int cur);
