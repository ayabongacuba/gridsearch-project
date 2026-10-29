# AI_CONTEXT.md — read this before helping with the project

Paste this whole file at the start of any conversation about this project.
It tells the AI tool who it is helping, what the project is, how the code
is laid out, what the marker checks, and what the tool may and may not do.

---

## 1. Who you are helping

A first-year university student in an introductory Data Structures and
Algorithms course, programming in C++.

**What they already know:** variables, functions, loops, `std::vector`,
`std::string`, classes with member functions, pointers, linked lists,
stacks, queues, binary trees. They have seen depth-first search and
breadth-first search in lectures, on small examples.

**What they have NOT met yet:** Dijkstra's algorithm, A\*, priority queues
and heaps, Floyd–Warshall, dynamic programming, the Travelling Salesman
Problem, big projects with many files, test frameworks, or trace recording.
Treat everything from Stage 3 onward as new. Explain it from the beginning,
in small steps, before writing code for it.

**Language.** Most students are not first-language English speakers. Use
short sentences and plain words. Say "the cell we are looking at" rather
than "the current node under consideration". Define any term the first
time you use it. Avoid idioms.

## 2. How to help

- Answer the question that was asked. Do not add material about other
  stages, the analysis, or the competition unless the student asks.
- Keep answers short. One idea at a time. A student can ask for more.
- When writing code: comment it. One short block comment at the top of
  each function saying what it does and what data structure holds the
  frontier, and a line comment wherever one of the rules in section 5 is
  applied. Comments are in plain English.
- Whole files are fine. If the student asks for a complete `dfs.cpp`, write
  the complete file. Explain it afterwards, in small pieces, so they can
  follow what each part does.
- If the student's code breaks a rule in section 5 or section 11, say so
  plainly and show the fix. Those rules decide the mark.
- Do not mention Phase 2 (the plots) or Phase 3 (the competition) unless
  the student brings them up. If they ask about plots, see section 12.
- Keep the log. The student must hand in `log.md`, a record of every
  time an AI tool was used on this project. Whenever you write or change
  code, end your answer with a ready-to-paste entry for `log.md` in this
  form, and if you can edit files directly, append it to `log.md` in the
  project root yourself:

  ```
  ## <date> — <tool name>
  Asked: <the request, in one line>
  Got:   <what was produced: which file, which function, roughly how many lines>
  Changed by me: <left blank for the student to fill in>
  ```

  The last line is the student's and you never fill it in. The log is
  read to see whether the student understood and corrected what they
  were given, so it must be accurate; do not shorten or tidy what was
  asked.

## 3. What the project is

A grid map of a university campus. Some cells are buildings (walls); the
rest are open ground with a cost to walk through. The student writes seven
C++ files, each one stage:

| Stage | File | What it does | Frontier structure |
| --- | --- | --- | --- |
| 0 | util.cpp | reads the map; the `Grid` class | a flat `std::vector`, one entry per cell |
| 1 | dfs.cpp | depth-first search: finds *a* route | stack |
| 2 | bfs.cpp | breadth-first search: fewest steps | queue |
| 3 | dijkstra.cpp | cheapest route when cells cost different amounts | priority queue (min-heap) |
| 4 | astar.cpp | same route as Dijkstra, looks at fewer cells | min-heap, ordered by g + h |
| 5 | floyd.cpp | the K×K table of costs between every pair of stops, two ways | a V×V table |
| 6 | tsp.cpp | the cheapest round trip through all stops, by trying every order | the K×K table + one permuted vector |

Every later stage calls Stage 0. Stages 5 and 6 call the student's own
Stage 3 Dijkstra.

The code is marked by automated tests that ship with the project, plus an
exact comparison of output against reference files. The route a search
returns, the order it commits to cells, the counts it reports and the
tour it produces must all match the reference **exactly**. That is only
possible because of the rules in section 5.

## 4. Where everything is

The project is a flat folder. `src/`, `include/` and the others sit at
the top level, not inside another folder.

```
include/     grid.h  search.h  matrix.h  trace.h      declarations — NEVER edit
src/         util.cpp dfs.cpp bfs.cpp dijkstra.cpp     the student writes these seven
             astar.cpp floyd.cpp tsp.cpp
             provided.cpp  main_*.cpp                  given — NEVER edit
tests/       the tests the marker runs                 NEVER edit
expected/    the reference output per stage per map    NEVER edit
maps/        26 maps        bench/   maps for the analysis (Phase 2 only)
run          build one stage and watch it
check        compare output with expected/
Makefile
```

Only the seven files in `src/` are submitted. The marker throws away
everything else and uses its own copies, so an edit anywhere else does
nothing except hide a problem.

## 5. The rules the marker depends on

These are not style preferences. Breaking any of them makes the output
differ from the reference and the exact-match tests fail.

1. **Neighbour order is N, E, S, W.** Up, right, down, left. Whenever code
   looks at the cells next to a cell, it looks in that order. In every
   stage. `DR[]` and `DC[]` in `grid.h` give the order; use them.
2. **Cost is charged when you ENTER a cell.** Moving from cell `a` to cell
   `b` costs `cost(b)`. The start cell is free. A route's cost is the sum
   of `cost()` over every cell after the start.
3. **Ties go to the smaller row, then the smaller column** (Stages 3
   and 4). When two frontier entries have the same priority, the one with
   the smaller row comes out first; if rows are equal, the smaller column.
4. **Lazy deletion** (Stages 3 and 4). `std::priority_queue` cannot change
   an entry already inside it. When a cheaper route to a cell is found,
   push a NEW entry. When an entry comes out and its cell is already
   settled, throw it away (and call `trace::discard`).
5. **Dijkstra orders by g** (cost so far). **A\* orders by g + h** where
   `h` is the Manhattan distance to the goal: `|row difference| +
   |column difference|`. Nothing else. No diagonal distance, no
   scaling, no other heuristic.
6. **Depth-first search is iterative**, with an explicit stack. Never
   recursive: one test map is a corridor 798 cells long and the marker
   runs tests under a reduced stack limit.
7. **Brute-force TSP** enumerates with `std::next_permutation` starting
   from `[1, 2, ..., K-1]` and replaces the best only when a candidate is
   **strictly** cheaper (`<`, not `<=`). This fixes which of several
   equally cheap tours is returned, and the marker compares it exactly.
8. **Floyd–Warshall's loop over the intermediate cell k is the OUTERMOST
   of the three loops.** Any other nesting is wrong in a way that looks
   right on small maps.

## 6. The map file

```
5 9            rows, then columns
#########      '#' is a building; it can never be entered
#...#..3#      '.' is open ground and costs 1 to enter
#.#.#.#.#      '1' to '9' is open ground that costs that digit to enter
#.#...#.#
#########
2              number of stops
1 1            stop 0: the start (row 1, column 1); also the depot for the tour
3 7            stop 1: the destination for stages 1 to 4
```

Rows count down from 0 at the top; columns count right from 0 at the
left. Movement is up, down, left, right only — never diagonal.

## 7. The types and function signatures (exactly as in include/)

```cpp
struct Cell { int r, c; };          // a position: row r, column c

// Neighbour order for the whole project: N, E, S, W.
inline constexpr int DR[4] = {-1, 0, 1, 0};
inline constexpr int DC[4] = { 0, 1, 0,-1};

extern const long INF;               // "no route"; defined by the student in util.cpp
                                     // must be well below LONG_MAX/2 so INF + INF does not overflow

class Grid {                         // declared in grid.h, IMPLEMENTED BY THE STUDENT in util.cpp
public:
    int rows, cols;
    std::vector<std::string> map;    // map[r][c] is '#', '.', or '1'..'9'
    std::vector<Cell> waypoints;     // waypoints[0] is the start / depot

    bool inBounds(int r, int c) const;           // inside the map at all?
    bool passable(int r, int c) const;           // inside AND not a '#'
    long cost(int r, int c) const;               // cost to ENTER this cell: 1 for '.', the digit otherwise
    int  index(int r, int c) const;              // r * cols + c  — use to index flat vectors
    int  index(Cell x) const;
    Cell at(int i) const;                        // the inverse of index
    std::vector<Cell> neighbours(Cell x) const;  // walkable neighbours, in N,E,S,W order
    Cell start() const;                          // waypoints[0]
    Cell goal()  const;                          // waypoints[1]
};

// Stage 0 also implements:
Grid readGrid(std::istream &in);                                  // parse the format above;
                                                                  //   bad row length or fewer than 1 stop
                                                                  //   -> print a message, std::exit(2)
std::vector<long> makeCostTable(int k);                           // k*k entries, all INF, 0 on the diagonal, row-major
std::vector<std::string> renderPath(const Grid &g, const std::vector<Cell> &path);
                                                                  // copy of the map with '^' '>' 'v' '<' along the
                                                                  //   route in the direction of travel, 'S' at the
                                                                  //   start, 'G' at the destination

// Stages 1 to 4: ONE function, the same signature in each file
struct Result {
    bool found;                      // false if no route exists (then path is empty)
    std::vector<Cell> path;          // the route: start first, destination last, adjacent steps, no repeats
    long cost;                       // sum of cost() over every cell in path AFTER the start
    long expanded;                   // how many cells were taken off the frontier and committed to
    long maxFrontier;                // the LARGEST number of entries the frontier ever held at once
};
Result search(const Grid &g, Cell start, Cell goal);
extern const char *ALGORITHM;        // each file defines it: "dfs", "bfs", "dijkstra" or "astar"

// Stage 5
struct Matrix {
    int k;                           // number of stops
    std::vector<long> d;             // k*k costs, row-major; at(i, j) reads d[i*k + j]
    long peakBytes;                  // size in bytes of the largest table allocated
    long long relaxations;           // how many times a table entry was improved (any honest count)
    long at(int i, int j) const;
};
Matrix waypointMatrix(const Grid &g, const std::vector<Cell> &stops, bool useFloyd);
//   useFloyd == true  : Floyd–Warshall over EVERY cell of the map, then read out the K stops
//   useFloyd == false : run the student's Stage 3 Dijkstra once from each stop
//   The two tables must be identical. Unreachable pairs are INF.

// Stages 6 and 7
struct Tour {
    std::vector<int> order;          // stop indices, starting AND ending at 0: e.g. 0 3 1 2 0
    long cost;                       // sum of the table entries along that order
    long long candidates;            // how many orders were examined
};
Tour bestTour(const Grid &g, const std::vector<Cell> &stops);
//   The student builds the K*K table (Stage 5) inside this function, then searches orders.
//   If any stop is unreachable, cost is INF and the driver prints NOTOUR.
```

## 8. The trace hooks — every search must call these

The visualiser replays them, and the marker checks them against the
numbers in `Result`. They are declared in `trace.h` and cost nothing when
tracing is off, so they stay in the code always.

```cpp
trace::push(r, c, g, f);       // a cell has just been added to the frontier
                               //   g = cost so far, f = its priority (same as g for DFS/BFS/Dijkstra)
trace::expand(r, c, pr, pc);   // a cell has just been taken off the frontier and committed to
                               //   (pr, pc) = the cell it was reached from; (-1, -1) for the start
trace::discard(r, c);          // an entry came off the frontier but was thrown away
                               //   (its cell was already visited / settled)
trace::relax(r, c, g, pr, pc); // a cheaper route to a cell that was ALREADY on the frontier was found
                               //   (happens in A*; in this cost model it does not happen in Dijkstra)
```

What the marker checks, and what each means for where the hooks go:

- Every `expand` of a cell must come after a `push` of that cell.
- No cell is expanded twice.
- The parent named in `expand` must itself have been expanded earlier.
- The running count (pushes − expands − discards) never goes below zero,
  and its peak must equal `Result.maxFrontier`.
- The number of `expand` calls must equal `Result.expanded`.
- The sequence of expanded cells must match the reference exactly.

So: `expanded` is counted when a cell comes OFF the frontier and is
committed to, not when it goes on. `maxFrontier` is measured from the
frontier's actual size (for lazy deletion that means queue ENTRIES,
including stale ones), not from the number of distinct cells seen.

## 9. Each stage: what matters, and what the tests check

**Stage 0, util.cpp.** `neighbours()` in N,E,S,W order is what every later
exact-order test depends on. `INF` must be large but not `LONG_MAX`: one
test checks `INF + INF > 0`. `readGrid` must detect a row of the wrong
length and a stop count under 1 and exit with code 2. Tests: parsing,
costs, neighbours and corners, `index`/`at` inverses, the cost table,
`renderPath` arrows.

**Stage 1, dfs.cpp.** Stack. Mark a cell visited when it is POPPED, not
pushed; a cell can be pushed more than once, and the second copy is
discarded when it surfaces (`trace::discard`). Push neighbours in N,E,S,W
order, so W is explored first. Tests: route is legal on every map;
reported cost equals the route's cost; no route on a sealed map
(`found == false`, empty path); destination equals start (path of one
cell, cost 0, expanded 1); destination one step away; a 798-cell
corridor; the recording is consistent; the exact route and the exact
commit order match the reference on every map.

**Stage 2, bfs.cpp.** Queue. Mark a cell seen when it is PUSHED. No cell
is ever queued twice; nothing is ever discarded. Tests: as Stage 1, plus
the step count equals the true minimum on every map, and `maxFrontier`
stays small on an open map (a ring, not the whole map) — this is the
test that catches marking on pop.

**Stage 3, dijkstra.cpp.** `std::priority_queue` of (cost, row, col)
with the tie-break in section 5, smallest first. Lazy deletion. Settle a
cell when it comes off; skip settled cells. Because cost is charged on
entry, every route into a cell pays the same for the last step, so the
first settled neighbour is always the cheapest way in: a correct Dijkstra
here produces NO relax events and NO discards, and every cell is pushed
exactly once. Tests: cheapest cost on every map including terrain; on
terrain maps the route is strictly longer in steps than breadth-first's
(a Dijkstra that returns the BFS answer fails); no relax, no discard;
cells come off in non-decreasing cost; exact route and order.

**Stage 4, astar.cpp.** Identical to Dijkstra except the priority is
`g + h`, `h` = Manhattan distance. Keep `dist[]` holding `g`. Here a cell
CAN be reached again more cheaply, so relax and discard DO occur. Tests:
same cost as Dijkstra on every map; never more cells expanded than
Dijkstra; strictly fewer on an open map (catches `h` always returning 0,
which is admissible but useless); Manhattan never exceeds the true
remaining cost; a room where two cells tie in the same row so only the
column rule decides; exact route and order.

**Stage 5, floyd.cpp.** Table of V×V longs, V = rows × cols, flat
vector. Initialise INF everywhere, 0 on the diagonal, `cost(v)` for each
adjacent pair (u, v). Triple loop with k outermost; skip when
`d[i][k] == INF` (also avoids INF + x overflow). Read out the K×K slice
for the stops. For `useFloyd == false`, call the student's own Dijkstra
from each stop. `peakBytes` for the Floyd path is at least 8·V². Tests:
both tables identical; both match an independent oracle; diagonal 0;
`d(i,j) − d(j,i) == cost(j) − cost(i)` for every pair; INF for sealed
stops, in both modes; a winding corridor that catches the wrong loop
order; `peakBytes` ≥ 8V²; repeated Dijkstra reports far less memory.

**Stage 6, tsp.cpp.** Build the table with `waypointMatrix(g, stops,
false)`. Then `std::vector<int> perm = {1, 2, ..., K-1}` and
`do { ... } while (std::next_permutation(...))`. Cost of an order = legs
from 0 through the permutation and back to 0; skip any order with an
INF leg. Keep strictly cheaper. Set `candidates` to the number of orders
examined. Tests: K = 1 (cost 0), K = 2, K = 3; the exact optimum and
the exact order on the shipped maps; an asymmetric table; a layout where
nearest-neighbour costs 50% more than optimal (a greedy shortcut fails);
tour starts and ends at 0; every stop once; reported cost equals the
legs; a sealed stop means INF.

## 10. Running and testing

All commands run from the project folder.

```
make                         build every stage
make test                    run every stage's tests
make test-dfs                one stage's tests (test-util, test-bfs, test-dijkstra, ...)
./bin/test_dfs "[order]"     one tagged group inside a stage
./bin/test_dfs --list-tests  the tags and names

./check                      compare every stage's output with expected/
./check dfs                  one stage
./check dfs m05              one map, showing which lines differ

./run dfs m05                build dfs, run it on map m05, open the replay in a browser
./run compare m05            all four searches side by side
./run astar t04 --render     print the map with the route drawn on it
./run floyd w02 --fw         Stage 5, Floyd–Warshall      (--dij for repeated Dijkstra)
./run tsp w02                Stage 6
```

Map names can be shortened to any unique prefix (`m05`, `t03`, `w02`).

If `./run` or `./check` says "Permission denied", the scripts lost their
execute bit when the archive was unpacked: `chmod +x run check` fixes it.
On Windows the project is used from inside WSL (Ubuntu), with the same
commands; `run.bat` exists only for a plain Windows prompt.

Reading a test failure: the line number is the assertion; the name above
it is the test; "with expansion" shows the two values compared. A message
like `commit 152 differs: the reference commits to (4,15) and you commit
to (4,17)` means the ordering rules were broken somewhere; look at the
neighbour order and the tie-break.

If the tests pass and `./check` is green, the stage is correct. If `check`
fails on the rendered route while the tests pass, an ordering rule was
broken.

## 11. What you must NOT do

- Edit, or suggest editing, anything outside the seven files in `src/`.
  Headers, drivers, tests, maps, expected output: never.
- Change a function signature, add a parameter, rename a function, or
  add a `main()`.
- Remove, move, or skip a trace hook; or compute `expanded` /
  `maxFrontier` from anything other than the frontier itself.
- Use any neighbour order, tie-break, or heuristic other than those in
  section 5.
- Write depth-first search recursively.
- Use threads, external libraries, or files from inside any stage.
  Standard library only.
- Write the student's analysis text, or the "Changed by me" line of a
  `log.md` entry. The rest of each entry is yours to write (section 2).
- Bring up Phase 2 or Phase 3 unless the student asks about them.

## 12. Later parts — only if the student asks

**Phase 2 (analysis).** The student runs `./run bench`, which measures
every stage on the maps in `bench/` and writes `bench/results.csv`, then
draws five plots (max frontier, time, A\* versus Dijkstra, route cost,
brute-force time against K) and writes short explanations with a
prediction marked on each. `bench/plot.py` draws the measurements; it is
a starting point they edit. Help with matplotlib syntax is fine. Do not
write their predictions or their explanations. Phase 2 is submitted
through a Moodle quiz: the plots are uploaded there and the follow-up
questions about them are answered there, in the student's own words.

**Phase 3 (competition).** A leaderboard with the same `bestTour`
signature on maps of 30 to 160 stops, a 2-second wall-clock limit per
map, and a gate requiring the exact optimum on the small maps. Writing a
complete solver is fine. The things to get right: keep a best-so-far tour
from the first moment and return it when the clock is nearly out; the
table is asymmetric because cost is charged on entry, so any swap that
reverses a segment must be re-priced in the new direction; and the Stage
6 brute force must still be used, or matched exactly, when K is small
enough for the gate.
