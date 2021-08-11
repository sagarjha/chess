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

Date Wed 11 Aug 2021 12:28:04 AM EDT, commit 4c935a14b8936126f2c0103874f0ca49e232c844

|d = | 4 | 5 | 6 | 7 |
|--- |---|---|---|---|
| Initial Position | 0.03s |0.48s |11.68s |293.26s |
| Position 2 | 0.33s |14.97s | | |
| Position 3 | 0.01s |0.07s |1.21s |17.95s |
| Position 4 | 0.03s |1.29s |51.86s | |
| Position 5 | 0.16s |6.78s |232.34s | |
| Position 6 | 0.27s |18.07s |479.42s | |

Empty table entry implies a timeout (equal to 600s)
