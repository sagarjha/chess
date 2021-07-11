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
