#include "position.hpp"

#include <algorithm>
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
            cur++;
        }
    }
    // not tracking pawn positions for now
    piece_pos[PAWN][0].clear();
    piece_pos[PAWN][1].clear();

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
    std::vector<move_t> moves;

    for(int cur = 0; cur < 64; ++cur) {
        int piece = board[cur];
        if(piece != EMPTY && PCOLOR(piece) == to_move) {
            std::vector<int> all_moves = get_piece_moves(board, cur);
            for(const int dest : all_moves) {
                // promotion
                if((piece / 2 == PAWN) && (dest < 8 || dest >= 56)) {
                    for(int promoted_piece = KNIGHT; promoted_piece <= QUEEN; ++promoted_piece) {
                        if(board[dest] == EMPTY) {
                            moves.push_back({{piece, {cur, GRAVE}}, {-1, {-1, -1}},
                                            {2 * promoted_piece + to_move, {GRAVE, dest}}});
                        } else {
                            moves.push_back({{piece, {cur, GRAVE}}, {board[dest], {dest, GRAVE}},
                                            {2 * promoted_piece + to_move, {GRAVE, dest}}});
            }
                    }
                } else{
                    if(board[dest] == EMPTY) {
                        moves.push_back({{piece, {cur, dest}}, {-1, {-1, -1}}, {-1, {-1, -1}}});
                    } else {
                        moves.push_back({{piece, {cur, dest}}, {board[dest], {dest, GRAVE}},
                                        {-1, {-1, -1}}});
                    }
                }
            }
        }
    }

    // enpassant
    if(enpassant_file != -1) {
        if(enpassant_file > 0 && board[MIDDLE_ROW(!to_move) + enpassant_file - 1] == 2 * PAWN + to_move) {
            moves.push_back({{2 * PAWN + to_move,
                             {MIDDLE_ROW(!to_move) + enpassant_file - 1, MIDDLE_ROW(!to_move) + PAWN_DIR(to_move) + enpassant_file}},
                            {2 * PAWN + !to_move, {MIDDLE_ROW(!to_move) + enpassant_file, GRAVE}},
                            {-1, {-1, -1}}});
        }
        if(enpassant_file < 7 && board[MIDDLE_ROW(!to_move) + enpassant_file + 1] == 2 * PAWN + to_move) {
            moves.push_back({{2 * PAWN + to_move,
                             {MIDDLE_ROW(!to_move) + enpassant_file + 1,
                              MIDDLE_ROW(!to_move) + PAWN_DIR(to_move) + enpassant_file}},
                            {2 * PAWN + !to_move, {MIDDLE_ROW(!to_move) + enpassant_file, GRAVE}},
                            {-1, {-1, -1}}});
        }
    }

    // castling
    if(castling_info[to_move][QUEENSIDE]
       && board[HOME_ROW(to_move) + 1] == EMPTY && board[HOME_ROW(to_move) + 2] == EMPTY
       && board[HOME_ROW(to_move) + 3] == EMPTY
       && !under_attack(HOME_ROW(to_move) + 2, !to_move) && !under_attack(HOME_ROW(to_move) + 3, !to_move)
       && !under_attack(HOME_ROW(to_move) + 4, !to_move)) {
        moves.push_back({{2 * KING + to_move, {HOME_ROW(to_move) + 4, HOME_ROW(to_move) + 2}},
                        {2 * ROOK + to_move, {HOME_ROW(to_move), HOME_ROW(to_move) + 3}},
                        {-1, {-1, -1}}});
    }
    if(castling_info[to_move][KINGSIDE]
       && board[HOME_ROW(to_move) + 5] == EMPTY && board[HOME_ROW(to_move) + 6] == EMPTY
       && !under_attack(HOME_ROW(to_move) + 4, !to_move) && !under_attack(HOME_ROW(to_move) + 5, !to_move)
       && !under_attack(HOME_ROW(to_move) + 6, !to_move)) {
        moves.push_back({{2 * KING + to_move, {HOME_ROW(to_move) + 4, HOME_ROW(to_move) + 6}},
                        {2 * ROOK + to_move, {HOME_ROW(to_move) + 7, HOME_ROW(to_move) + 5}},
                        {-1, {-1, -1}}});
    }

    memory_t m;
    for(int color : {WHITE, BLACK}) {
        for(int side : {KINGSIDE, QUEENSIDE}) {
	    m.old_castling_info[color][side] = castling_info[color][side];
	}
    }
    m.old_enpassant_file = enpassant_file;

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
    
    if((main_piece / 2 == PAWN) && std::abs(main_end - main_start) == 16) {
        enpassant_file = main_end % 8;
    } else {
        enpassant_file = -1;
    }

    if(is_promo) {
        board[main_start] = EMPTY;
	board[move.promo.second.second] = move.promo.first;
    } else if(is_castling) {
        board[main_start] = EMPTY;
        board[main_end] = main_piece;
	board[move.side.second.first] = EMPTY;
	board[move.side.second.second] = move.side.first;
    } else {
        board[main_start] = EMPTY;
        board[main_end] = main_piece;
        if(is_enpassant) {
	    board[move.side.second.first] = EMPTY;
        }
    }

    std::function<void(int, int, int)> change_pos = [this](int piece, int past, int cur) {
        if(piece == -1 || piece / 2 == PAWN) {
            return;
        }

        if(cur == -1) {
            std::vector<int>& vec = piece_pos[piece / 2][PCOLOR(piece)];
            vec.erase(std::remove(vec.begin(), vec.end(), past), vec.end());
	    return;
        }

        if(past == -1) {
            piece_pos[piece / 2][PCOLOR(piece)].push_back(cur);
	    return;
        }

	std::vector<int>& vec = piece_pos[piece / 2][PCOLOR(piece)];
	std::replace(vec.begin(), vec.end(), past, cur);
    };

    change_pos(main_piece, main_start, main_end);
    change_pos(move.side.first, move.side.second.first, move.side.second.second);
    change_pos(move.promo.first, move.promo.second.first, move.promo.second.second);

    to_move = !to_move;

    // checks
    if(under_attack(piece_pos[KING][!to_move][0], to_move)) {
        throw "ILLEGAL";
    }
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
        board[main_start] = main_piece;
        if(was_capture) {
            board[move.promo.second.second] = move.side.first;
        } else {
            board[move.promo.second.second] = EMPTY;	  
	}
    } else if(was_castling) {
        board[main_start] = main_piece;
        board[main_end] = EMPTY;
	board[move.side.second.first] = move.side.first;
	board[move.side.second.second] = EMPTY;
    } else {
        board[main_start] = main_piece;
        if(was_capture) {
            if(was_enpassant) {
	        board[main_end] = EMPTY;
		board[move.side.second.first] = move.side.first;
            } else {
                board[main_end] = move.side.first;
            }
        } else {
            board[main_end] = EMPTY;
        }
    }

    std::function<void(int, int, int)> revert_pos = [this](int piece, int past, int cur) {
        if(piece == -1 || piece / 2 == PAWN) {
            return;
        }

        if(cur == -1) {
            piece_pos[piece / 2][PCOLOR(piece)].push_back(past);
            return;
        }
	
        if(past == -1) {
            std::vector<int>& vec = piece_pos[piece / 2][PCOLOR(piece)];
            vec.erase(std::remove(vec.begin(), vec.end(), cur), vec.end());
	    return;
        }

	std::vector<int>& vec = piece_pos[piece / 2][PCOLOR(piece)];
	std::replace(vec.begin(), vec.end(), cur, past);
    };

    revert_pos(main_piece, main_start, main_end);
    revert_pos(move.side.first, move.side.second.first, move.side.second.second);
    revert_pos(move.promo.first, move.promo.second.first, move.promo.second.second);

    to_move = !to_move;
}

bool position::under_attack(const int square, const bool whom) {
    int original_piece = board[square];

    std::function<bool(int)> test = [this, original_piece, square, whom](int piece) {
        board[square] = 2 * piece + !whom;
        std::vector<int> moves = get_piece_moves(board, square);
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

    std::function<bool(int)> rook_check = [&square, this, &whom](const int enemy_square) {
        if(square / 8 == enemy_square / 8) {
            for(int i = std::min(square, enemy_square) + 1; i < std::max(square, enemy_square); ++i) {
                if(board[i] != EMPTY) {
                    return false;
                }
            }
            return true;
        }
        if(square % 8 == enemy_square % 8) {
            for(int i = std::min(square, enemy_square) + 8; i < std::max(square, enemy_square); i += 8) {
                if(board[i] != EMPTY) {
		    return false;
                }
            }
            return true;
        }
	return false;
    };

    for(int enemy_square : piece_pos[ROOK][whom]) {
        if(rook_check(enemy_square)) {
            return true;
        }
    }

    for(int enemy_square : piece_pos[QUEEN][whom]) {
        if(rook_check(enemy_square)) {
            return true;
        }
    }

    std::function<bool(int)> bishop_check = [&square, this, &whom](const int enemy_square) {
        if(std::abs(square / 8 - enemy_square / 8) == std::abs(square % 8 - enemy_square % 8)) {
            int dist = std::abs(square - enemy_square) / std::abs(square / 8 - enemy_square / 8);
            for(int i = std::min(square, enemy_square) + dist; i < std::max(square, enemy_square); i += dist) {
                if(board[i] != EMPTY) {
		    return false;
                }
            }
            return true;
        }
        return false;
    };

    for(int enemy_square : piece_pos[BISHOP][whom]) {
        if(bishop_check(enemy_square)) {
            return true;
        }
    }

    for(int enemy_square : piece_pos[QUEEN][whom]) {
        if(bishop_check(enemy_square)) {
            return true;
        }
    }

    int enemy_square = piece_pos[KING][whom][0];
    int diff = std::abs(enemy_square - square);
    if((diff >= 7 && diff <= 9) || (diff == 1)) {
        if(std::abs((square % 8) - (enemy_square % 8)) <= 1) {
            return true;
        }
    }

    if(square / 8 != HOME_ROW(whom) / 8 && test(PAWN)) {
        return true;
    }

    return false;
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
        std::cout << (char)('a' + enpassant_file) << 8 - (MIDDLE_ROW(!to_move) + PAWN_DIR(to_move))/8;
    }

    std::cout << " 0 1";

    std::cout << std::endl;
}
