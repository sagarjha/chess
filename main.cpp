#include <iostream>

#include "position.hpp"
#include "types.hpp"

uint64_t i = 0;

void explore(position& pos, int total_ply, int cur_ply) {
    if(cur_ply == total_ply) {
        i++;
        // pos.print();
        return;
    }
    
    std::vector<position> next_positions = pos.next_positions();
    for(position& next : next_positions) {
        explore(next, total_ply, cur_ply + 1);
    }
}

int main() {
    position pos;
    pos.initialize();
    pos.print();

    int ply = 3;
    explore(pos, ply, 0);
    std::cout << i << std::endl;
}
