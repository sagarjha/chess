#include <iostream>

#include <chess/position.hpp>
#include <chess/piece.hpp>
#include <chess/types.hpp>

uint64_t i = 0;

void explore(position& pos, int total_ply, int cur_ply) {
    if(cur_ply == total_ply) {
        i++;
        // pos.print();
        // std::cout << std::endl;
        return;
    }
    
    std::vector<position> next_positions = pos.next_positions();
    for(position& next : next_positions) {
        explore(next, total_ply, cur_ply + 1);
    }
}

void explore2(position& pos, int ply) {
    int total = 0;
    std::vector<position> next_positions = pos.next_positions();
    for(position& next_pos : next_positions) {
        i = 0;
        next_pos.print();
	explore(next_pos, ply - 1, 0);
        total += i;
        std::cout << i << std::endl;
    }
    std::cout << "total = " << total << std::endl;
}

int main(int, char* argv[]) {
    position pos(argv[1]);

    int ply = std::stoi(argv[2]);

    explore(pos, ply, 0);
    std::cout << i << std::endl;

    // explore2(pos, ply);
}
