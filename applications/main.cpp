#include <iostream>

#include <chess/position.hpp>
#include <chess/types.hpp>

uint64_t i = 0;

void explore(position& pos, int total_ply, int cur_ply) {
    if(cur_ply == total_ply) {
        i++;
        // pos.print_trace();
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
    	next_pos.print_trace();
	explore(next_pos, ply - 1, 0);
        total += i;
        std::cout << i << std::endl;
    }
    std::cout << "total = " << total << std::endl;
}

int main(int, char* argv[]) {
    // "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
    // "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
    // "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    // "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    position pos(argv[1]);

    int ply = std::stoi(argv[2]);

    explore(pos, ply, 0);
    std::cout << i << std::endl;

    // explore2(pos, ply);
}
