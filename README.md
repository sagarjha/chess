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

Date Wed 11 Aug 2021 05:53:15 AM EDT, commit 564def7170ce5217e21e845c07bcdfb36dfe41e2

|d = | 4 | 5 | 6 | 7 |
|--- |---|---|---|---|
| Initial Position | 0.02s |0.33s |8.03s |210.74s |
| Position 2 | 0.25s |10.94s |481.63s | |
| Position 3 | 0.00s |0.06s |1.10s |15.47s |
| Position 4 | 0.03s |0.91s |41.56s | |
| Position 5 | 0.12s |5.35s |185.08s | |
| Position 6 | 0.20s |8.65s |352.23s | |

Empty table entry implies a timeout (equal to 600s)
