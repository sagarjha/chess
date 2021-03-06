#include <iomanip>
#include <iostream>
#include <chrono>

#include <chess/piece.hpp>
#include <chess/position.hpp>
#include <chess/types.hpp>

uint64_t i = 0;

void explore(position& pos, int total_ply, int cur_ply = 0) {
    if(cur_ply == total_ply) {
        i++;
        // pos.print();
        // std::cout << std::endl;
        return;
    }

    std::pair<std::pair<std::vector<move_t>, std::vector<move_t>>, memory_t> moves_plus_memory = pos.get_moves();
    for(move_t& move : moves_plus_memory.first.first) {
        pos.make_king_move(move);
        explore(pos, total_ply, cur_ply + 1);
        pos.take_king_back(move, moves_plus_memory.second);
    }
    for(move_t& move : moves_plus_memory.first.second) {
        pos.make_move(move);
        explore(pos, total_ply, cur_ply + 1);
        pos.take_back(move, moves_plus_memory.second);
    }
}

void explore2(position& pos, int ply) {
    int total = 0;
    std::pair<std::pair<std::vector<move_t>, std::vector<move_t>>, memory_t> moves_plus_memory = pos.get_moves();
    for(move_t& move : moves_plus_memory.first.first) {
        pos.make_king_move(move);
        i = 0;
        pos.print_fen();
        explore(pos, ply - 1);
        pos.take_king_back(move, moves_plus_memory.second);
        total += i;
        std::cout << i << std::endl;
    }
    for(move_t& move : moves_plus_memory.first.second) {
        pos.make_move(move);
        i = 0;
        pos.print_fen();
        explore(pos, ply - 1);
        pos.take_back(move, moves_plus_memory.second);
        total += i;
        std::cout << i << std::endl;
    }
    std::cout << "total = " << total << std::endl;
}

int main(int, char* argv[]) {
    position pos(argv[1]);

    int ply = std::stoi(argv[2]);

    int print_total = std::stoi(argv[3]);
    int print_time = std::stoi(argv[4]);

    // pos.print();

    auto start_time = std::chrono::steady_clock::now();

    explore(pos, ply);

    auto end_time = std::chrono::steady_clock::now();
    long long int nanoseconds_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

    if(print_total) {
        std::cout << i << std::endl;
    }
    
    if(print_time) {
        std::cout << std::fixed << std::setprecision(2)
                  << nanoseconds_elapsed / 1e9 << "s" << std::endl;
    }

    // explore2(pos, ply);
}
