# bench/ -- Phase 2

Every plot in the analysis is made on these maps and no others, so that
plots are comparable between students and a reader knows what N or K a
point refers to.

    s11 s15 s21 s25 s31 s41     square open terrain maps, 8 stops each
                                N (open cells) = 74, 156, 336, 496, 788, 1421
                                V (all cells)  = 121, 225, 441, 625, 961, 1681
    k04 ... k15                 ONE 31x31 grid, with the first K of 15 stops

    ./run bench                 measures everything -> bench/results.csv
    ./run bench --quick         without k13 and k14
    python3 bench/plot.py       draws the five plots -> bench/plots/

    Plot 1   maxFrontier, DFS and BFS, against N; plus m02 for contrast
    Plot 2   time: the four searches against N; Floyd-Warshall vs repeated Dijkstra against V
    Plot 3   expanded, Dijkstra against A*, against N
    Plot 4   route cost, all four searches, against N
    Plot 5   brute-force time against K, k04 to k14

results.csv columns: map, N, V, K, algorithm, expanded, max_frontier, cost,
steps, time_us, peak_kb, candidates. Timings are the median of seven runs.
