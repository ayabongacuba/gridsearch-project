# Gridsearch — The Travelling Computer Scientist

A C++17 project for exploring pathfinding and multi-stop route planning on weighted grid maps, with interactive visualisations, automated tests, and performance benchmarks.

Gridsearch models a map as a graph: walls block movement, open cells have entry costs, and waypoints mark locations to visit. It implements several search and optimisation algorithms so their routes, work, and trade-offs can be inspected and compared.

## Features

- **Four pathfinding algorithms:** Depth-First Search (DFS), Breadth-First Search (BFS), Dijkstra's algorithm, and A*.
- **Waypoint cost matrices:** Floyd–Warshall and repeated Dijkstra searches.
- **Multi-stop route planning:** exact travelling-salesperson search for smaller waypoint sets, with a nearest-neighbour and 2-opt heuristic for larger sets.
- **Interactive visualisations:** browser replays, live search traces, side-by-side algorithm comparisons, and waypoint-tour views.
- **Automated tests:** Catch2 tests plus output comparisons against reference results.
- **Benchmark tools:** scripts for measuring search work, execution time, memory use, and tour candidates.

## Algorithms

| Algorithm | Purpose |
|---|---|
| Depth-First Search (DFS) | Finds a valid route using a stack. It does not guarantee the shortest or cheapest route. |
| Breadth-First Search (BFS) | Finds a route with the fewest steps when each move is treated equally. |
| Dijkstra | Finds a minimum-cost route when cells have different entry costs. |
| A* | Uses a Manhattan-distance heuristic to guide a minimum-cost search towards the goal. |
| Floyd–Warshall | Computes shortest-path costs between all grid cells, then extracts the waypoint-to-waypoint costs. |
| Repeated Dijkstra | Builds the waypoint cost matrix by running Dijkstra searches between waypoint pairs. |
| Travelling Salesperson | Finds a closed tour that starts at waypoint 0, visits every other waypoint once, and returns to the start. |

For tours with up to 11 waypoints, the implementation checks every possible ordering. For larger waypoint sets, it uses a multi-start nearest-neighbour construction followed by 2-opt local improvement. The larger-instance method is a heuristic and is not guaranteed to find the global optimum.

## The Grid Model

A map is a rectangular grid with four-way movement (north, east, south, and west). Diagonal movement is not allowed.

| Map character | Meaning | Cost to enter |
|---|---|---:|
| `#` | Wall; cannot be entered | — |
| `.` | Open cell | 1 |
| `1`–`9` | Open cell with a terrain cost | The displayed digit |

Movement cost is charged when entering a cell. The starting cell is not charged. This cost model is also why the waypoint cost matrix can be asymmetric: travelling from waypoint A to B does not necessarily have the same cost as travelling from B to A.

Map files contain the grid dimensions, the rows of the map, and then a list of waypoint coordinates. Waypoint 0 is the start/depot; waypoint 1 is the goal for the single-destination searches.

## Repository Structure

| Path | Purpose |
|---|---|
| `include/` | Shared data structures and function interfaces. |
| `src/` | Grid utilities, search algorithms, route-planning algorithms, and command-line drivers. |
| `maps/` | Example grid maps used by the programs. |
| `tests/` | Automated tests and test support. |
| `expected/` | Reference outputs used by the comparison script. |
| `web/` | HTML and JavaScript visualisation templates. |
| `bench/` | Benchmark inputs, measurement script, and plotting script. |
| `.vscode/` | VS Code build, test, and debugging configurations. |
| `Makefile` | Build and test targets. |
| `run` | Builds and runs an algorithm on a selected map. |
| `check` | Compares program output with the reference results. |

For implementation details and design trade-offs, see [DESIGN.md](DESIGN.md).

## Requirements

On Ubuntu or another Linux distribution:

- A C++17-compatible compiler (GCC or Clang).
- GNU Make.
- A web browser for the HTML visualisations.

Install the core tools on Ubuntu:

```bash
sudo apt update
sudo apt install build-essential g++ make
```

Optional tools:

```bash
# Debugging from VS Code
sudo apt install gdb

# Benchmark plots
sudo apt install python3 python3-matplotlib

# Native visualisation window (the browser visualiser works without SDL2)
sudo apt install pkg-config libsdl2-dev
```

SDL2 is optional. Without it, requests for a native window fall back to the browser visualisation.

## Build

Clone the repository and enter its directory:

```bash
git clone https://github.com/ayabongacuba/gridsearch-project.git
cd gridsearch-project
```

Build all algorithm executables:

```bash
make
```

The executables are written to `bin/`.

Build one algorithm only:

```bash
make astar
```

Remove generated executables and replay files:

```bash
make clean
```

## Run

The `run` script builds the requested executable and runs it on a map. By default, it opens a browser replay.

```bash
# Run a search and open its replay
./run dfs m05

# Run without opening a visualisation
./run bfs m05 --no-viz

# Run a weighted search
./run dijkstra t02

# Run A* and print the route on the map
./run astar t04 --render

# Compare DFS, BFS, Dijkstra, and A* on the same map
./run compare m05

# Build a waypoint matrix using Floyd–Warshall
./run floyd w02 --fw

# Build the same matrix using repeated Dijkstra searches
./run floyd w02 --dij

# Find a closed tour through the waypoints
./run tsp w02
```

Map names can be shortened to a unique prefix, such as `m05`, `t04`, or `w02`. Run `./run --help` to see the available maps and options.

### Visualisation modes

- **Browser replay:** the default mode. The generated HTML file is saved in `runs/` and opened in a browser.
- **Live replay:** `./run dfs m05 --live` opens the browser first and streams search events while the algorithm runs.
- **Native window:** add `--window` when SDL2 is installed.
- **Side-by-side comparison:** `./run compare m05` replays the four single-goal search algorithms in synchronisation by expanded cells.

## Tests and Reference Checks

Run the automated tests:

```bash
# All tests
make test

# Tests for one algorithm
make test-dfs
make test-astar
```

Run the reference-output checks:

```bash
# Check every algorithm and map
./check

# Check one algorithm
./check astar

# Check one algorithm on one map
./check astar m05
```

The test suite checks algorithm behaviour and edge cases. The `check` script compares the program's output with the reference files in `expected/`, while ignoring values that naturally vary between runs, such as execution time.

## Benchmarks

The benchmark script measures search behaviour and route-planning performance:

```bash
# Run the full benchmark set
./run bench

# Run a shorter set
./run bench --quick

# Generate plots from bench/results.csv
python3 bench/plot.py
```

The measurements are written to `bench/results.csv`; the plots are written to `bench/plots/`. The plotting script requires Matplotlib.

## Visual Studio Code

Open the workspace file:

```bash
code project.code-workspace
```

Install Microsoft's **C/C++** extension to use IntelliSense and the provided debugger configurations. The workspace tasks include building the project, running tests, launching an algorithm, and checking reference output. GDB is required for the Linux debugging configuration.

## Licence

This repository does not currently include an open-source licence. Until a licence is added, do not assume that the code is available for reuse, modification, or redistribution.
