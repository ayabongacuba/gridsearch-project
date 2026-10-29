# Testing your work

The same tests we mark with are in the handout. Run them as often as you like.

```
make test            every stage
make test-util       stage 0        make test-astar     stage 4
make test-dfs        stage 1        make test-floyd     stage 5
make test-bfs        stage 2        make test-tsp       stage 6
make test-dijkstra   stage 3
```

Nothing to install. Catch2 is a single header sitting in `tests/`. Its
`main` is compiled once into `bin/catch_main.o` and linked into every test
binary, so the first `make test` is slow and the rest are quick.

## Reading a failure

```
-------------------------------------------------------------------------------
the reported cost equals the cost of the route printed
-------------------------------------------------------------------------------
tests/test_dfs.cpp:88
...............................................................................

tests/test_dfs.cpp:94: FAILED:
  REQUIRE( r.cost == sum )
with expansion:
  132 == 138
```

The line number is the assertion, the name above it is the test, and the
expansion shows the two values it compared. Some failures come with a
sentence instead:

```
tests/test_dfs.cpp:170: FAILED:
  REQUIRE( checkRecording(g, r, rec) == "" )
with expansion:
  "you reported MAXFRONTIER 0 but the recording peaks at 1598"
```

That one means your hooks and your counters disagree: the recording says the
frontier reached 1598 entries and your `Result` claims it never grew at all.

## Running a subset

While you are working on one thing, run only that thing:

```
./bin/test_dfs "[route]"     just the route-legality tests
./bin/test_dfs "[cost]"      just the arithmetic
./bin/test_dfs "[edge]"      just the awkward maps
./bin/test_dfs "[trace]"     just the recording
./bin/test_dfs "[order]"     just the exact expansion sequence
```

Or name a single test:

```
./bin/test_dfs "the destination is one step away"
```

Useful flags:

```
./bin/test_dfs -s            show passing assertions too, not just failures
./bin/test_dfs --list-tests  print every test name and its tags
./bin/test_dfs -b            stop at the first failure
```

## What each stage's tests check

**Stage 0, util** — `[parse]` a map reads back exactly as written; `[cost]`
open ground is 1 and digits are their value; `[nbr]` neighbours come back
N,E,S,W and corners return only what exists; `[matrix]` the table starts
empty with a zero diagonal; `[render]` arrows point the way you travelled.

**Stage 2, bfs** — everything Stage 1 required, plus `[optimal]` the step
count equals the true minimum on every map, and `[frontier]` the frontier
stays a ring rather than flooding. That second group is the one that catches
marking cells as seen when you pop them instead of when you push them: the
route is still correct, so only the frontier numbers give it away.

**Stage 3, dijkstra** — `[optimal]` the cheapest cost on every map, and on
the terrain maps a route that is *strictly longer in steps* than the
fewest-steps route. `[order]` the tie-break rule, tested on an open room
where all four neighbours cost the same and only the rule decides. There is
also a test asserting your search produces **no** relax events and **no**
discards, which surprises people — see the comment on it.

**Stage 4, astar** — `[optimal]` the same cost as Dijkstra everywhere;
`[work]` fewer cells committed to, including a strict bound that catches an
estimate which always returns zero (that estimate is admissible and never
expands more than Dijkstra, so the obvious two tests both pass it);
`[heuristic]` Manhattan distance checked directly against true remaining
cost over many sampled cells.

**Stage 5, floyd** — `[agree]` the two methods produce identical tables;
`[correct]` both match an independent oracle, the table is asymmetric by
exactly the end costs, unreachable stops are INF; `[fw]` a route needing
several intermediate cells, and a winding corridor that catches the k loop
being nested in the wrong place; `[mem]` the reported table size is at least
8V².

**Stage 6, tsp** — `[tiny]` one, two and three stops; `[exact]` the true
optimum on the shipped maps, on an asymmetric table, and on a layout where
going to the nearest unvisited stop costs 50% more than optimal;
`[structure]` the tour starts and ends at the depot and visits every stop
once; `[edge]` a sealed stop means no tour.

## What the Stage 1 groups check

**`[route]`** — the walk you return is legal: it starts and ends in the right
places, every step is to an orthogonally adjacent cell, it never enters a
building, and it never visits the same cell twice.

**`[cost]`** — the number you report is the cost of the route you printed,
and cost is charged for entering a cell rather than leaving one. There is
also a test asserting your route is *never shorter* than the true optimum,
which is there to document that depth-first search is allowed to be bad.

**`[edge]`** — no route exists; the destination is where you already are; the
destination is one step away; a map with a single open cell; a corridor 798
cells long. If that last one crashes rather than fails, your search is
recursive.

**`[trace]`** — the recording your hooks produced is internally consistent.
Every cell committed to joined the frontier first. No cell is committed to
twice. A cell's parent was committed to before the cell itself. The frontier
count never goes negative. The number of commits equals your `expanded`, and
the peak frontier size derived from the recording equals your `maxFrontier`.
There is also a check that stale entries are being discarded, on a map where
the reference discards 126 of them.

**`[order]`** — the sequence of cells you commit to, compared against the
reference, exactly. This is only possible because the neighbour order is
fixed at N, E, S, W. Get that wrong and these fail while everything else
passes, which is a useful signal in itself.

## Passing is not the same as being finished

These tests check that your code is *correct*. They cannot check that you
understand it, and the marks are not distributed as though they could. Before
you move on, run `./run dfs m05` and `./run dfs m06` and watch what the
search actually does on each. If you cannot say why the two look so
different, you are not done.
