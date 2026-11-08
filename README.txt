COP4634 Project 3 - Hungry Lizard Crossing
Team: Lance Penaflor (LP), Jay McIntosh (JM)

Extra credit COMPLETED: unidirectional travel.
  - lizards.cpp    : bidirectional solution (UNIDIRECTIONAL 0, WORLDEND 180)
  - lizardsUni.cpp : unidirectional solution (UNIDIRECTIONAL 1, WORLDEND 180)
    Lizards crossing in one direction block lizards wanting the opposite direction
    until the driveway empties. A turnstile semaphore prevents starvation.

Build:  make        (builds both; "make clean" removes binaries)
Run:    ./lizards [-d]     ./lizardsUni [-d]
