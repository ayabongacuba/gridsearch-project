# Design

This document describes the problem model, the main components, and the trade-offs behind Gridsearch.

## 1. Problem Model

Gridsearch represents a map as a weighted graph.

Each passable grid cell is a vertex. An edge connects two vertices when their cells are orthogonally adjacent. Movement is limited to north, east, south, and west; diagonal movement is not allowed.

The map uses these characters:

- `#`: blocked cell.
- `.`: passable cell with an entry cost of 1.
- `1` to `9`: passable cell whose entry cost is the represented digit.

The cost of a route is the sum of the costs of the cells entered after leaving the starting cell. The start itself is free.

This is an important modelling choice. Although movement between adjacent cells is possible in either direction, the cost of entering the destination cell makes the graph's path costs directional. The cost from waypoint A to waypoint B can therefore differ from the cost from B to A.

A map also contains an ordered list of waypoints. Waypoint 0 is the starting point and, for a closed tour, the depot. Waypoint 1 is the destination for the single-goal searches.

## 2. Project Architecture

The project is divided into a small number of layers.

### Interfaces — `include/`

The headers define the data structures and functions shared by the program:

- `grid.h` defines cells, the grid, movement directions, and map utilities.
- `search.h` defines the result returned by a single-goal search.
- `matrix.h` defines waypoint cost matrices and closed tours.
- `trace.h` defines the events recorded while a search runs.
- `viz.h` and `window.h` provide the visualisation interfaces.

The implementation files use these interfaces so that algorithms can be built and tested through common drivers.

### Implementations and drivers — `src/`

The source directory contains the grid utilities, the search and optimisation algorithms, and the command-line drivers.

The single-goal algorithms share `main_search.cpp`. The waypoint matrix and tour algorithms use `main_matrix.cpp` and `main_tour.cpp` respectively. Shared map-reading and path-validation helpers are provided separately.

### Tests and reference data — `tests/` and `expected/`

The Catch2 test suite checks routes, costs, edge cases, and algorithm-specific behaviour. Reference output files provide a second check for the results produced on the supplied maps.

### Visualisation — `web/`

The HTML and JavaScript files render search traces, compare several searches, and display waypoint matrices and tours. The browser replay is generated from the recorded trace and saved as an HTML file under `runs/`.

## 3. Single-Goal Search Algorithms

The project implements four ways to search from a start cell to a goal cell.

### Depth-First Search (DFS)

DFS uses a stack. It explores one branch of the map before returning to explore another. It can find a valid route, but the route is not guaranteed to have the fewest steps or the lowest cost.

### Breadth-First Search (BFS)

BFS uses a queue and explores the map in layers of increasing step count. When every move has the same cost, BFS finds a route with the fewest steps. On maps with different terrain costs, the route with the fewest steps is not necessarily the cheapest route.

### Dijkstra's Algorithm

Dijkstra uses a priority queue ordered by the total cost accumulated so far. It finds a minimum-cost route when movement costs are non-negative.

### A* Search

A* uses the same accumulated route cost as Dijkstra, with an additional estimate of the remaining distance to the goal. This implementation uses Manhattan distance:

`h(n) = |row(n) - row(goal)| + |column(n) - column(goal)|`

Because each move costs at least 1 and movement is four-directional, Manhattan distance does not overestimate the remaining cost. It can guide the search towards the goal while preserving minimum-cost results.

## 4. Waypoint Cost Matrices

For multi-stop planning, the program first needs the cost of travelling between each pair of waypoints.

The result is a K by K matrix, where entry `(i, j)` stores the cost from waypoint `i` to waypoint `j`.

The project provides two approaches:

- **Floyd–Warshall:** computes all-pairs shortest-path costs over the grid cells, then extracts the waypoint-to-waypoint entries. If the grid has V cells, this method uses a V by V distance table.
- **Repeated Dijkstra:** runs a shortest-path search for each ordered pair of distinct waypoints and stores the resulting costs in the K by K matrix.

The approaches produce the same waypoint costs but have different time and memory requirements. Floyd–Warshall uses O(V²) space and O(V³) time. Repeated Dijkstra avoids the V by V table but repeats searches for waypoint pairs.

### Why the Matrix Can Be Asymmetric

The cost to travel from A to B includes the cost of entering B, while the reverse trip includes the cost of entering A. The starting waypoint is not charged.

As a result, for two mutually reachable waypoints, the difference between the two directions is determined by their endpoint entry costs:

`d(A, B) - d(B, A) = cost(B) - cost(A)`

The matrix driver checks this relationship.

## 5. Travelling-Salesperson Tour

The tour starts at waypoint 0, visits every other waypoint exactly once, and returns to waypoint 0.

For up to 11 waypoints, the implementation fixes waypoint 0 and evaluates every ordering of the remaining waypoints. This is an exact search, but the number of possible orders grows factorially.

For larger waypoint sets, the implementation uses a heuristic:

1. Build a nearest-neighbour tour from several starting waypoints.
2. Apply 2-opt local improvements to the tour using a symmetric adjustment of the waypoint costs.
3. Evaluate each candidate using the original directed waypoint cost matrix.
4. Keep the best tour found.

The larger-instance approach trades a guarantee of global optimality for a practical search over more waypoints. It can produce a good tour, but it does not guarantee the cheapest possible tour.

## 6. Search Traces and Visualisation

The algorithms record events as they work. The trace interface includes events for:

- A cell being added to the frontier.
- A cell being expanded.
- A frontier entry being discarded.
- A cheaper route to a cell being found.

The browser visualiser uses these events to replay the search. It can show the map, the frontier, the expansion order, and summary statistics. The comparison view advances multiple algorithms in step with one another by expansion count, rather than by wall-clock time.

The waypoint visualiser displays the cost matrix and the legs of a closed tour.

## 7. Validation and Measurement

Gridsearch uses two complementary forms of validation.

- **Automated tests** check properties such as route legality, route cost, edge cases, and algorithm-specific behaviour.
- **Reference-output checks** compare results against stored outputs for the supplied maps. Runtime-dependent fields, such as timings, are excluded from the comparison.

The benchmark tools collect measurements such as expanded cells, maximum frontier size, route cost, execution time, memory estimates, and the number of tour candidates. These measurements can be plotted to compare the algorithms and their trade-offs.

## 8. Known Trade-offs

- DFS is useful for demonstrating stack-based exploration, but it does not guarantee an optimal route.
- BFS minimises the number of steps, not terrain cost.
- Dijkstra and A* account for terrain costs.
- A*'s heuristic guides the search, but the amount of work it saves depends on the map.
- Floyd–Warshall's V by V table can require substantial memory on large grids.
- Exact tour enumeration becomes impractical as the number of waypoints grows, which is why larger instances use a heuristic.
