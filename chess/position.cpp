#include "position.hpp"

#include <algorithm>
#include <chrono>
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
	    cur += (ch - '0');
        } else {
            int piece = char_to_piece(ch);
            board.place(cur, piece);
            cur++;
        }
    }
    fen_pos++;
    if(fen[fen_pos] == 'w') {
        to_move = WHITE;
    } else {
        to_move = BLACK;
    }
    fen_pos += 2;

    castling_info[WHITE][QUEENSIDE] = castling_info[WHITE][KINGSIDE] = false;
    castling_info[BLACK][QUEENSIDE] = castling_info[BLACK][KINGSIDE] = false;
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

std::pair<std::vector<move_t>, memory_t> position::get_moves() {
    memory_t m;
    for(int color : {WHITE, BLACK}) {
        for(int side : {KINGSIDE, QUEENSIDE}) {
            m.old_castling_info[color][side] = castling_info[color][side];
        }
    }
    m.old_enpassant_file = enpassant_file;

    board.reset_pins_map();

    std::vector<move_t> moves;

    int in_check_from[2];
    board.find_checks(in_check_from, to_move);
    // special case - king is in check
    if(in_check_from[0] != -1) {
      int piece = board[in_check_from[0]];
      board.find_king_moves(moves, to_move);
      if(in_check_from[1] == -1) {
          board.find_captures(moves, in_check_from[0]);
          if(board[in_check_from[0]] / 2 == PAWN && in_check_from[0] % 8 == enpassant_file) {
              board.find_enpassant_captures(moves, to_move, enpassant_file);
          }
          if(piece / 2 == ROOK || piece / 2 == BISHOP || piece / 2 == QUEEN) {
              board.find_blocking_moves(moves, in_check_from[0], to_move);
          }
      }

      return {moves, m};
    }

    for(int cur = 0; cur < 64; ++cur) {
        int piece = board[cur];
        if(piece != EMPTY && PCOLOR(piece) == to_move) {
            int pin_dir = board.check_pinned(cur);
            if(pin_dir != -1) {
                if(!(piece / 2 == KNIGHT || (piece / 2 == ROOK && pin_dir >= 2)
                     || (piece / 2 == BISHOP && pin_dir < 2))) {
                    if(piece / 2 == PAWN) {
                        if(pin_dir != 0) {
                            board.find_pawn_moves(moves, cur, pin_dir, enpassant_file);
                        }
                    } else {
                        board.find_piece_moves(moves, cur, pin_dir);
                    }
                }
                continue;
            }

            if(piece / 2 == PAWN) {
                board.find_pawn_moves(moves, cur, enpassant_file);
            } else if(piece / 2 == KING) {
                board.find_king_moves(moves, to_move, castling_info[to_move]);
            } else {
                board.find_piece_moves(moves, cur);
            }
        }
    }
    
    return {moves, m};
}

void position::make_move(const move_t move) {
    bool is_castling = (move.side.first != -1) && (PCOLOR(move.main.first) == PCOLOR(move.side.first));
    bool is_capture = !is_castling && (move.side.first != -1);
    bool is_promo = (move.promo.second.second != -1);
    bool is_enpassant = is_capture && (move.main.second.second != move.side.second.first);

    int main_piece = move.main.first;
    int main_start = move.main.second.first;
    int main_end = move.main.second.second;

    int capture_start = move.side.second.first;
    if(is_capture && (capture_start % 8 == 0 || capture_start % 8 == 7)) {
        castling_info[!to_move][QUEENSIDE] &= !(capture_start == HOME_ROW(!to_move) + 0);
        castling_info[!to_move][KINGSIDE] &= !(capture_start == HOME_ROW(!to_move) + 7);
    }

    if(main_piece / 2 == KING) {
        castling_info[to_move][QUEENSIDE] = castling_info[to_move][KINGSIDE] = false;
    }

    if(main_piece / 2 == ROOK) {
        castling_info[to_move][QUEENSIDE] &= !(main_start == HOME_ROW(to_move) + 0);
        castling_info[to_move][KINGSIDE] &= !(main_start == HOME_ROW(to_move) + 7);
    }

    if(main_piece / 2 == PAWN && std::abs(main_end - main_start) == 16) {
        enpassant_file = main_end % 8;
    } else {
        enpassant_file = -1;
    }

    if(is_promo) {
        board.remove(main_start);
	board.remove(move.promo.second.second);
	board.place(move.promo.second.second, move.promo.first);
    } else if(is_castling) {
        board.replace(main_start, main_end);
	board.replace(move.side.second.first, move.side.second.second);
    } else {
	board.replace(main_start, main_end);
        if(is_enpassant) {
            board.remove(move.side.second.first);
        }
    }

    to_move = !to_move;
}

void position::take_back(const move_t move, const memory_t m) {
    for(int color : {WHITE, BLACK}) {
        for(int side : {KINGSIDE, QUEENSIDE}) {
            castling_info[color][side] = m.old_castling_info[color][side];
        }
    }
    enpassant_file = m.old_enpassant_file;

    bool was_castling = (move.side.first != -1) && (PCOLOR(move.main.first) == PCOLOR(move.side.first));
    bool was_capture = !was_castling && (move.side.first != -1);
    bool was_promo = (move.promo.second.second != -1);
    bool was_enpassant = was_capture && (move.main.second.second != move.side.second.first);

    int main_piece = move.main.first;
    int main_start = move.main.second.first;
    int main_end = move.main.second.second;
    
    if(was_promo) {
        board.place(main_start, main_piece);
        board.remove(move.promo.second.second);
        if(was_capture) {
            board.place(move.promo.second.second, move.side.first);
        }
    } else if(was_castling) {
        board.replace(main_end, main_start);
	board.replace(move.side.second.second, move.side.second.first);
    } else {
        board.replace(main_end, main_start);
        if(was_capture) {
            if(was_enpassant) {
                board.place(move.side.second.first, move.side.first);
            } else {
	      board.place(main_end, move.side.first);
            }
        }
    }
    
    to_move = !to_move;
}

void position::print() const {
    std::cout << (to_move == WHITE ? "W" : "B") << std::endl;
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            std::cout << piece_to_char(board[i * 8 + j]);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void position::print_fen() const {
    for(int i = 0; i < 8; ++i) {
        int num_empty = 0;
        for(int j = 0; j < 8; ++j) {
            if(board[8 * i + j] == EMPTY) {
                num_empty++;
            } else {
                if(num_empty != 0) {
                    std::cout << num_empty;
                    num_empty = 0;
                }
                std::cout << piece_to_char(board[i * 8 + j]);
            }
        }
        if(num_empty != 0) {
            std::cout << num_empty;
            num_empty = 0;
        }
        if(i != 7) {
            std::cout << "/";
        } else {
            std::cout << " ";
        }
    }

    std::cout << (to_move == WHITE ? 'w' : 'b');
    std::cout << " ";

    if(castling_info[WHITE][KINGSIDE]) {
        std::cout << 'K';
    }
    if(castling_info[WHITE][QUEENSIDE]) {
        std::cout << 'Q';
    }
    if(castling_info[BLACK][KINGSIDE]) {
        std::cout << 'k';
    }
    if(castling_info[BLACK][QUEENSIDE]) {
        std::cout << 'q';
    }

    if(!castling_info[WHITE][KINGSIDE] && !castling_info[WHITE][QUEENSIDE]
       && !castling_info[BLACK][KINGSIDE] && !castling_info[BLACK][QUEENSIDE]) {
        std::cout << "-";
    }

    std::cout << " ";

    if(enpassant_file == -1) {
        std::cout << "-";
    } else {
        std::cout << (char)('a' + enpassant_file) << 8 - (MIDDLE_ROW(!to_move) + PAWN_DIR(to_move)) / 8;
    }

    std::cout << " 0 1";

    std::cout << std::endl;
}
