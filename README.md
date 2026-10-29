# gridsearch — The Travelling Computer Scientist

The travelling computer scientist. Seven stages, built on each other.

```
make                 build every stage
make test            run every test
./check              diff your output against expected/
./run dfs m05        build, run, and watch it
```

## What you write

| stage | file | task |
|---|---|---|
| 0 | `src/util.cpp` | the map itself: `class Grid`, `readGrid`, `makeCostTable`, `renderPath`, `INF` |
| 1 | `src/dfs.cpp` | find a route |
| 2 | `src/bfs.cpp` | find the shortest route |
| 3 | `src/dijkstra.cpp` | find the cheapest route |
| 4 | `src/astar.cpp` | find it faster |
| 5 | `src/floyd.cpp` | the cost between every pair of stops, two ways |
| 6, 7 | `src/tsp.cpp` | the cheapest round trip |

Signatures are declared in `include/`. Stages 1–4 implement
`Result search(const Grid&, Cell, Cell)`; Stage 5 implements
`waypointMatrix`; Stages 6 and 7 implement `bestTour`.

Everything links against your Stage 0, and Stages 5–7 also link against your
Stage 3. A bug low down breaks everything above it.

## Running

```
./run dfs                    default map, browser replay
./run bfs m05                any unique prefix of a map name
./run dfs m07 --window       native window (needs SDL2)
./run dfs m05 --live         browser opens first, search streams into it
./run compare m05            all four side by side, stepped together
./run astar t04 --render     also print the map with the route on it
./run dijkstra t02 --no-viz  numbers only
./run floyd w02 --fw         stage 5, Floyd-Warshall
./run floyd w02 --dij        stage 5, repeated Dijkstra
./run tsp w02                stage 6
```

Replays are saved to `runs/` as standalone HTML files.

## Checking your work

```
make test              every stage's tests
make test-dfs          one stage
./bin/test_dfs "[order]"   one group within a stage

./check                every stage against expected/
./check dfs            one stage
./check dfs m05        one map, showing which lines differ
```

`expected/` holds two kinds of file per search stage per map. The `.out`
files are what the program prints; `check` diffs against those, ignoring
lines that legitimately vary between runs — timings, memory figures, how many
orderings you examined. The `.seq` files are the full commit order, one cell
per line. They never reach the program's output, so `check` can't see them;
the `[order]` tests read them instead and tell you exactly which commit first
disagrees.

`check` and the tests catch different things. Hooks in the wrong place with
correct counters pass `check` and fail the tests. A tie-break that's right on
a small room but wrong somewhere on a big map fails both `check` and the
full-order test.

## The maps

`m01`–`m09` uniform cost, Stages 1–2. `t01`–`t05` terrain costs, Stages 3–4.
`w01`–`w07` multiple stops, Stages 5–7. `e01`–`e05` edge cases: a sealed
destination, a 798-cell corridor, a destination one step away, a start that
is already the destination, a map with one open cell.

`./run bench` measures everything Phase 2 needs into `bench/results.csv`, and `python3 bench/plot.py` draws it; see `bench/README.md`. `AI_CONTEXT.md` is what to give any AI tool before asking it about this project.

`map_gen.py` makes more.

## Requirements

A C++17 compiler and `make`. A browser for the replays, which are plain files
on disk with no server behind them. SDL2 is optional and only needed for
`--window`.

## VS Code

Open `project.code-workspace`. Tasks are wired to whichever `src/*.cpp`
you have open: build it, test it, run it, check it. `F5` debugs the current
stage; the second launch configuration debugs its tests.
