#include "position.hpp"

#include <functional>
#include <iostream>

#include "piece.hpp"

position::position() : position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
}

position::position(const std::string& fen) {
    size_t fen_pos = 0;
    for(int cur = 0; cur < 64 && fen_pos < fen.size(); fen_pos++) {
        const char ch = fen[fen_pos];
        if(ch == '/') {
            continue;
        }
        if(std::isdigit(ch)) {
            for(int i = 0; i < ch - '0'; ++i) {
                board[cur++] = EMPTY;
            }
        } else {
            board[cur] = char_to_piece(ch);
	    piece_pos[board[cur] / 2][PCOLOR(board[cur])].push_back(cur);

            if(ch == 'K') {
                king_pos[WHITE] = cur;
            } else if(ch == 'k') {
                king_pos[BLACK] = cur;
            }
            cur++;
        }
    }
    piece_pos[PAWN][0].clear();
    piece_pos[PAWN][1].clear();
    piece_pos[KING][0].clear();
    piece_pos[KING][1].clear();

    fen_pos++;
    if(fen[fen_pos] == 'w') {
        to_move = WHITE;
    } else {
        to_move = BLACK;
    }
    fen_pos += 2;

    castling_info[WHITE] = {false, false};
    castling_info[BLACK] = {false, false};
    if(fen[fen_pos] == '-') {
        fen_pos++;
    } else {
        for(; fen[fen_pos] != ' '; ++fen_pos) {
            switch(fen[fen_pos]) {
                case 'K':
                    castling_info[WHITE][KINGSIDE] = true;
                    break;
                case 'Q':
                    castling_info[WHITE][QUEENSIDE] = true;
                    break;
                case 'k':
                    castling_info[BLACK][KINGSIDE] = true;
                    break;
                case 'q':
                    castling_info[BLACK][QUEENSIDE] = true;
                    break;
            }
        }
    }
    fen_pos++;

    if(fen[fen_pos] == '-') {
        enpassant_file = -1;
        fen_pos++;
    } else {
        enpassant_file = fen[fen_pos] - 'a';
        fen_pos += 2;
    }
}

position::position(const position& pos, const int start, const int end) {
    std::copy(std::begin(pos.board), std::end(pos.board), board);
    king_pos = pos.king_pos;
    castling_info = pos.castling_info;
    for(int piece = KNIGHT; piece <= QUEEN; ++piece) {
      piece_pos[piece][WHITE] = pos.piece_pos[piece][WHITE];
      piece_pos[piece][BLACK] = pos.piece_pos[piece][BLACK];
    }

    const int piece = board[start];
    
    castling_info[WHITE][QUEENSIDE] &= !(end == 56);
    castling_info[WHITE][KINGSIDE] &= !(end == 63);
    castling_info[BLACK][QUEENSIDE] &= !(end == 0);
    castling_info[BLACK][KINGSIDE] &= !(end == 7);
    
    if(piece/2 == KING) {
        castling_info[pos.to_move] = {false, false};
        king_pos[pos.to_move] = end;
    }

    if(piece/2 == ROOK) {
        castling_info[pos.to_move][QUEENSIDE] &= !(start == HOME_ROW(pos.to_move) + 0);
        castling_info[pos.to_move][KINGSIDE] &= !(start == HOME_ROW(pos.to_move) + 7);
    }

    board[end] = board[start];
    board[start] = EMPTY;

    if((piece/2 == PAWN) && std::abs(end - start) == 16) {
        enpassant_file = end % 8;
    } else {
        enpassant_file = -1;
    }

    if(piece / 2 != PAWN && piece / 2 != KING) {
        for(int& p : piece_pos[piece / 2][pos.to_move]) {
            if(p == start) {
                p = end;
                break;
            }
        }
    }

    int enemy_piece = pos.board[end];
    if(enemy_piece != EMPTY && enemy_piece / 2 != PAWN && enemy_piece / 2 != KING) {
        auto& vec = piece_pos[enemy_piece / 2][!pos.to_move];
        vec.erase(std::remove(vec.begin(), vec.end(), end), vec.end());
    }

    to_move = !pos.to_move;
}

position::position(const position& pos, const bool castle_side) {
    std::copy(std::begin(pos.board), std::end(pos.board), board);
    king_pos = pos.king_pos;
    castling_info = pos.castling_info;
    for(int piece = KNIGHT; piece <= QUEEN; ++piece) {
      piece_pos[piece][WHITE] = pos.piece_pos[piece][WHITE];
      piece_pos[piece][BLACK] = pos.piece_pos[piece][BLACK];
    }

    castling_info[pos.to_move][QUEENSIDE] = false;
    castling_info[pos.to_move][KINGSIDE] = false;
    
    if(castle_side == QUEENSIDE) {
        board[HOME_ROW(pos.to_move) + 2] = 2 * KING + pos.to_move;
        board[HOME_ROW(pos.to_move + 4)] = EMPTY;
        king_pos[pos.to_move] = HOME_ROW(pos.to_move) + 2;

        board[HOME_ROW(pos.to_move) + 3] = 2 * ROOK + pos.to_move;
        board[HOME_ROW(pos.to_move)] = EMPTY;

        for(int& p : piece_pos[ROOK][pos.to_move]) {
            if(p == HOME_ROW(pos.to_move)) {
                p = HOME_ROW(pos.to_move) + 3;
                break;
            }
        }
    } else {
        board[HOME_ROW(pos.to_move) + 6] = 2 * KING + pos.to_move;
        board[HOME_ROW(pos.to_move) + 4] = EMPTY;
        king_pos[pos.to_move] = HOME_ROW(pos.to_move) + 6;

        board[HOME_ROW(pos.to_move) + 5] = 2 * ROOK + pos.to_move;
        board[HOME_ROW(pos.to_move) + 7] = EMPTY;

        for(int& p : piece_pos[ROOK][pos.to_move]) {
            if(p == HOME_ROW(pos.to_move) + 7) {
                p = HOME_ROW(pos.to_move) + 5;
                break;
            }
        }
    }

    enpassant_file = -1;
    to_move = !pos.to_move;
}

std::vector<position> position::next_positions() {
    std::vector<position> positions;

    for(int cur = 0; cur < 64; ++cur) {
        int piece = board[cur];
        if(piece != EMPTY && PCOLOR(piece) == to_move) {
            std::vector<int> all_moves = get_moves(board, cur);
            for(const int dest : all_moves) {
                positions.emplace_back(*this, cur, dest);

                // promotion
                if((piece / 2 == PAWN) && (dest < 8 || dest >= 56)) {
                    position pos = positions.back();
                    positions.pop_back();
                    for(int piece = KNIGHT; piece <= QUEEN; ++piece) {
                        pos.board[dest] = 2 * piece + to_move;
			pos.piece_pos[piece][to_move].push_back(dest);
                        positions.push_back(pos);
			pos.piece_pos[piece][to_move].pop_back();
                    }
                }
            }
        }
    }

    // enpassant
    if(enpassant_file != -1) {
      if(enpassant_file > 0 && board[MIDDLE_ROW(!to_move) + enpassant_file - 1] == 2 * PAWN + to_move) {
          positions.emplace_back(*this,
                                 MIDDLE_ROW(!to_move) + enpassant_file - 1,
                                 MIDDLE_ROW(!to_move) + PAWN_DIR(to_move) + enpassant_file);
          positions.back().board[MIDDLE_ROW(!to_move) + enpassant_file] = EMPTY;
        }
        if(enpassant_file < 7 && board[MIDDLE_ROW(!to_move) + enpassant_file + 1] == 2 * PAWN + to_move) {
            positions.emplace_back(*this,
                                   MIDDLE_ROW(!to_move) + enpassant_file + 1,
                                   MIDDLE_ROW(!to_move) + PAWN_DIR(to_move) + enpassant_file);
            positions.back().board[MIDDLE_ROW(!to_move) + enpassant_file] = EMPTY;
        }
    }

    // castling
    if(castling_info[to_move][QUEENSIDE]
       && board[HOME_ROW(to_move) + 1] == EMPTY && board[HOME_ROW(to_move) + 2] == EMPTY
       && board[HOME_ROW(to_move) + 3] == EMPTY
       && !under_attack(HOME_ROW(to_move) + 2, !to_move) && !under_attack(HOME_ROW(to_move) + 3, !to_move)
       && !under_attack(HOME_ROW(to_move) + 4, !to_move)) {
        positions.emplace_back(*this, QUEENSIDE);
    }
    if(castling_info[to_move][KINGSIDE]
       && board[HOME_ROW(to_move) + 5] == EMPTY && board[HOME_ROW(to_move) + 6] == EMPTY
       && !under_attack(HOME_ROW(to_move) + 4, !to_move) && !under_attack(HOME_ROW(to_move) + 5, !to_move)
       && !under_attack(HOME_ROW(to_move) + 6, !to_move)) {
        positions.emplace_back(*this, KINGSIDE);
    }

    // checks
    positions.erase(std::remove_if(positions.begin(), positions.end(), [](position& pos) {
                        return pos.under_attack(pos.king_pos[!pos.to_move], pos.to_move);
                    }),
                    positions.end());

    return positions;
}

bool position::under_attack(const int square, bool whom) {
    int original_piece = board[square];

    std::function<bool(int)> test = [this, original_piece, square, whom](int piece) {
        board[square] = 2 * piece + !whom;
        std::vector<int> moves = get_moves(board, square);
        board[square] = original_piece;
        for(const int p : moves) {
            if(board[p] == 2 * piece + whom) {
                return true;
            }
        }
        return false;
    };

    for(int enemy_square : piece_pos[KNIGHT][whom]) {
        int diff = std::abs(enemy_square - square);
        if(diff == 6 || diff == 10 || diff == 15 || diff == 17) {
            if(std::abs((square % 8) - (enemy_square % 8)) <= 2) {
                return true;
            }
        }
    }

    for(int enemy_square : piece_pos[ROOK][whom]) {
        if(square / 8 == enemy_square / 8) {
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + 1; i < std::max(square, enemy_square); ++i) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
        if(square % 8 == enemy_square % 8) {
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + 8; i < std::max(square, enemy_square); i += 8) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
    }

    for(int enemy_square : piece_pos[BISHOP][whom]) {
        if(std::abs(square / 8 - enemy_square / 8) == std::abs(square % 8 - enemy_square % 8)) {
	    int dist = std::abs(square - enemy_square) / std::abs(square / 8 - enemy_square / 8);
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + dist; i < std::max(square, enemy_square); i += dist) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
    }

    for(int enemy_square : piece_pos[QUEEN][whom]) {
        if(square / 8 == enemy_square / 8) {
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + 1; i < std::max(square, enemy_square); ++i) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
        if(square % 8 == enemy_square % 8) {
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + 8; i < std::max(square, enemy_square); i += 8) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
    }


    for(int enemy_square : piece_pos[QUEEN][whom]) {
        if(std::abs(square / 8 - enemy_square / 8) == std::abs(square % 8 - enemy_square % 8)) {
	    int dist = std::abs(square - enemy_square) / std::abs(square / 8 - enemy_square / 8);
            bool all_empty = true;
            for(int i = std::min(square, enemy_square) + dist; i < std::max(square, enemy_square); i += dist) {
                if(board[i] != EMPTY) {
                    all_empty = false;
                }
            }
            if(all_empty) {
                return true;
            }
        }
    }

    int enemy_square = king_pos[whom];
    int diff = std::abs(enemy_square - square);
    if((diff >=7 && diff <= 9) || (diff == 1)) {
        if(std::abs((square % 8) - (enemy_square % 8)) <= 1) {
            return true;
        }
    }

    if(square/8 != HOME_ROW(whom)/8 && test(PAWN)) {
        return true;
    }

    return false;
}

void position::print() const {
    std::cout << (to_move == WHITE ? "W" : "B") << std::endl;
    for(int x = 0; x < 8; ++x) {
        for(int y = 0; y < 8; ++y) {
            std::cout << piece_to_char(board[x * 8 + y]);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int i = KNIGHT; i <= QUEEN; ++i) {
      for(int pos : piece_pos[i][WHITE]) {
          std::cout << pos << " ";
      }
      std::cout << std::endl;

      for(int pos : piece_pos[i][BLACK]) {
          std::cout << pos << " ";
      }
      std::cout << std::endl;
    }
}
