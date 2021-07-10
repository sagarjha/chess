#include "position.hpp"

#include <iostream>

#include "piece.hpp"

std::vector<position> position::next_positions() {
  std::vector<position> positions;
  
  point cur(0,0);
  for (const std::array<char, 8>& ranks : board) {
    for (const char piece : ranks) {
        if (piece && (side_to_move == get_color(piece))) {
	    std::vector<point> all_moves = get_moves(board, cur, piece);
            for(const point move : all_moves) {
                positions.push_back(generate_position(cur, move));
	    }
	}
        ++cur;
    }
  }

  // special moves
  if (en_passant_square.front != -1) {
      
  }

  return positions;
}

void position::initialize() {
  side_to_move = true;
  en_passant_square = {-1, -1};

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
    std::cout << std::endl;
}

position position::generate_position(const point start, const point end) {
    position new_pos;
    new_pos.board = board;
    
    new_pos.board[end.x][end.y] = new_pos.board[start.x][start.y];
    new_pos.board[start.x][start.y] = 0;

    new_pos.side_to_move = !side_to_move;

    return new_pos;
}
