# Installation
To compile in Release mode, run in the project directory:
* `mkdir Release`
* `cd Release`
* `cmake -DCMAKE_BUILD_TYPE=Release ..`
* `make`

Replace Release by Debug to compile in Debug mode.

# Testing
## Move Generation
From the Release directory, run:
* `cd bin`
* `./main <fen> <ply>`

# Results
## Move Generation
In reference to [Perft Results](https://www.chessprogramming.org/Perft_Results)

Date Wed 11 Aug 2021 12:19:05 AM EDT, commit 4bdd62d56f0ed73473991e6ac0ccd3d1b12d6744

|d = | 4 | 5 | 6 |
|--- |---|---|---|
| Initial Position | 0.04s |0.47s |11.58s |
| Position 2 | 0.34s |14.68s | |
| Position 3 | 0.01s |0.07s |1.15s |
| Position 4 | 0.03s |1.28s |51.60s |
| Position 5 | 0.16s |6.76s | |
| Position 6 | 0.26s |12.10s | |

Empty table entry implies a timeout (equal to 60s)
