#!/bin/bash

cat <<EOF > ../README.md
# Installation
To compile in Release mode, run in the project directory:
* \`mkdir Release\`
* \`cd Release\`
* \`cmake -DCMAKE_BUILD_TYPE=Release ..\`
* \`make\`

Replace Release by Debug to compile in Debug mode.

# Testing
## Move Generation
From the Release directory, run:
* \`cd bin\`
* \`./main <fen> <ply>\`

# Results
## Move Generation
In reference to [Perft Results](https://www.chessprogramming.org/Perft_Results)

Date `date`, commit `git rev-parse HEAD`

|d = | 4 | 5 | 6 | 7 |
|--- |---|---|---|---|
EOF

fens=(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
)

prompts=(
    "Initial Position"
    "Position 2"
    "Position 3"
    "Position 4"
    "Position 5"
    "Position 6"
)

num_pos=${#fens[@]}

timeout=600

for (( pos = 0; pos < num_pos; ++pos )); do
    fen=${fens[pos]}
    str="| ${prompts[pos]} | "
    for (( i = 4; i <= 7; ++i )); do
	time=`timeout $timeout ../Release/bin/main "$fen" $i 0 1`
	str+="$time"
	str+=" |"
    done
    echo $str >> ../README.md
done
echo >> ../README.md

echo "Empty table entry implies a timeout (equal to ${timeout}s)" >> ../README.md
