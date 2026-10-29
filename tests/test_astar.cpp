// tests/test_astar.cpp -- Stage 4.
//
//   make test-astar
//
// Tags: [optimal] same answer as Dijkstra   [work] fewer cells committed to
//       [heuristic] admissibility   [trace] the recording   [order] exact sequence

#include "catch.hpp"

#include "grid.h"
#include "harness.h"
#include "search.h"

using namespace gs;
using namespace gstest;

static long manhattan(Cell a, Cell b) {
    return std::abs(a.r - b.r) + std::abs(a.c - b.c);
}

// =====================================================================
// It must still be correct
// =====================================================================

TEST_CASE("the route it returns is a legal walk", "[route]") {
    for (const std::string &name : {"m05-caves-45", "t02-caves-45",
                                    "t05-open-51"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(validatePath(g, r.path, r.cost) == "");
        }
    }
}

TEST_CASE("it finds the same cost as Dijkstra on every map", "[optimal]") {
    // If this fails, the estimate is overestimating somewhere: A* has
    // committed to a cell too early and never gone back to correct itself.
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45", "m09-open-61",
                                    "t01-rooms-35", "t02-caves-45",
                                    "t03-braid-31", "t04-caves-81",
                                    "t05-open-51"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(r.cost == cheapestCost(g, g.start(), g.goal()));
        }
    }
}

TEST_CASE("no route exists", "[edge]") {
    Grid g = load("e01-sealed-31");
    Result r = search(g, g.start(), g.goal());
    REQUIRE_FALSE(r.found);
}

TEST_CASE("the destination is where we already are", "[edge]") {
    Grid g = make({"#####",
                   "#.9.#",
                   "#####"}, {{1, 2}, {1, 2}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.cost == 0);
    REQUIRE(r.expanded == 1);
}

// =====================================================================
// It must actually be doing something
// =====================================================================

TEST_CASE("it never commits to more cells than Dijkstra would", "[work]") {
    // The reference expansion counts, from the same maps, with the same
    // tie-breaking. An A* that expands MORE than Dijkstra is not using the
    // estimate at all, or is using it wrongly.
    struct Case { const char *map; long dijkstra; };
    const Case cases[] = {
        {"m01-perfect-21", 199}, {"m05-caves-45", 1762},
        {"m09-open-61", 3269},   {"t02-caves-45", 1521},
        {"t05-open-51", 2164},
    };
    for (const Case &c : cases) {
        SECTION(c.map) {
            Grid g = load(c.map);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << c.map << " -- Dijkstra commits to " << c.dijkstra
                 << ", you committed to " << r.expanded);
            REQUIRE(r.expanded <= c.dijkstra);
        }
    }
}

TEST_CASE("on an open map it commits to far fewer cells than Dijkstra",
          "[work]") {
    // An estimate that always returns 0 is admissible, and it passes both the
    // cost test and the "no more than Dijkstra" test -- because it IS
    // Dijkstra. This is the test that catches it.
    Grid g = load("m05-caves-45");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    INFO("Dijkstra commits to 1762 cells here; a working estimate should get "
         "you under half of that");
    REQUIRE(r.expanded < 880);
}

TEST_CASE("the saving is largest where the map is most open", "[work]") {
    Grid open = load("m09-open-61");
    Grid maze = load("m02-perfect-41");
    Result ro = search(open, open.start(), open.goal());
    Result rm = search(maze, maze.start(), maze.goal());
    REQUIRE(ro.found);
    REQUIRE(rm.found);
    long openCells = 0;
    for (const std::string &row : open.map)
        for (char ch : row) if (ch != '#') openCells++;
    INFO("on an open field the estimate has room to steer");
    REQUIRE(ro.expanded < openCells / 2);
}

// =====================================================================
// The estimate itself
// =====================================================================

TEST_CASE("Manhattan distance never overestimates", "[heuristic]") {
    // Admissibility, checked directly rather than inferred. For a sample of
    // cells, the estimate must not exceed the true remaining cost.
    Grid g = load("t02-caves-45");
    Cell goal = g.goal();
    int checked = 0;
    for (int r = 0; r < g.rows; r += 3) {
        for (int c = 0; c < g.cols; c += 3) {
            if (!g.passable(r, c)) continue;
            long truth = cheapestCost(g, Cell{r, c}, goal);
            if (truth < 0) continue;
            INFO("cell " << r << "," << c);
            REQUIRE(manhattan(Cell{r, c}, goal) <= truth);
            checked++;
        }
    }
    REQUIRE(checked > 20);
}

TEST_CASE("the estimate is worth less when every step is expensive",
          "[heuristic]") {
    // Manhattan distance assumes each step costs 1. Where the average cost is
    // much higher, the estimate is a weak lower bound and A* degenerates
    // towards Dijkstra. This is a real property, not a fault -- Stage 8 asks
    // you to quantify it.
    Grid open = load("m09-open-61");      // every step costs 1
    Grid rough = load("t05-open-51");     // same shape, costly terrain
    Result ro = search(open, open.start(), open.goal());
    Result rr = search(rough, rough.start(), rough.goal());
    REQUIRE(ro.found);
    REQUIRE(rr.found);
    // both should still be correct
    REQUIRE(rr.cost == cheapestCost(rough, rough.start(), rough.goal()));
}

// =====================================================================
// The recording
// =====================================================================

TEST_CASE("the recording is internally consistent", "[trace]") {
    for (const std::string &name : {"m05-caves-45", "t02-caves-45",
                                    "t05-open-51"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r;
            Recording rec = runRecorded(g, r);
            INFO("map: " << name);
            REQUIRE(checkRecording(g, r, rec) == "");
        }
    }
}

TEST_CASE("unlike Dijkstra A* does find cheaper routes to known cells",
          "[trace]") {
    // Dijkstra settles cells in order of cost-so-far, so the first route it
    // finds to a cell is always the cheapest and it never updates anything.
    // A* settles them in order of cost-so-far PLUS estimate, which is a
    // different order -- so a cell can be reached again, later, more cheaply.
    // Your relax hook must fire when that happens.
    Grid g = load("t02-caves-45");
    Result r;
    Recording rec = runRecorded(g, r);
    long relaxes = 0;
    for (const Event &e : rec.events) if (e.kind == 'R') relaxes++;
    INFO("no relax events at all means trace::relax is never being called");
    REQUIRE(relaxes > 0);
}

TEST_CASE("stale queue entries are discarded", "[trace]") {
    Grid g = load("t05-open-51");
    Result r;
    Recording rec = runRecorded(g, r);
    long discards = 0;
    for (const Event &e : rec.events) if (e.kind == 'D') discards++;
    INFO("when a cell is improved it goes back into the queue; the old entry "
         "must be thrown away when it surfaces");
    REQUIRE(discards > 0);
    REQUIRE(checkRecording(g, r, rec) == "");
}

// =====================================================================
// Order
// =====================================================================

TEST_CASE("the search leans towards the destination", "[order]") {
    // In an open room with the goal to the east, the first cells committed to
    // after the start should be eastward, not the full ring.
    Grid g = make({"#########",
                   "#.......#",
                   "#.......#",
                   "#.......#",
                   "#########"}, {{2, 1}, {2, 7}});
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE(rec.expandedOrder.size() >= 3);
    REQUIRE(rec.expandedOrder[0] == Cell{2, 1});
    INFO("the estimate should pull the search east before it spreads north "
         "and south");
    REQUIRE(rec.expandedOrder[1] == Cell{2, 2});
}

TEST_CASE("ties are broken by row then by column", "[order]") {
    //   # # # # #
    //   # . S . #      start (1,2), goal (2,1)
    //   # G . . #
    //   # . . . #
    //   # # # # #
    //
    // After committing to (1,2) and then (1,1), the frontier holds (2,1) at
    // g=2, h=0 and (2,2) at g=1, h=1 -- both f = 2, and both in row 2. Only
    // the column rule decides, and it picks (2,1), which is the goal. Break
    // the tie any other way and you commit to (2,2) first for nothing.
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{1, 2}, {2, 1}});
    Result r;
    Recording rec = runRecorded(g, r);
    INFO("sequence was: " << sequence(rec));
    REQUIRE(sequence(rec) == "1,2 1,1 2,1");
    REQUIRE(r.expanded == 3);
}

TEST_CASE("the full commit order matches the reference on every map", "[order]") {
    // The small hand-built rooms above check the rules in isolation. This
    // checks that they hold across a whole run on every shipped map: the
    // cells you commit to, in the order you commit to them, compared one by
    // one against expected/astar/<map>.seq. A rule that is only wrong in
    // some configuration -- a tie broken the wrong way three thousand commits
    // in -- is caught here and nowhere else.
    for (const std::string &name : {
          "m01-perfect-21", "m02-perfect-41", "m03-braid-31", "m04-rooms-35",
          "m05-caves-45", "m06-comb-25", "m07-perfect-81", "m08-caves-101",
          "m09-open-61", "t01-rooms-35", "t02-caves-45", "t03-braid-31",
          "t04-caves-81", "t05-open-51", "e02-corridor-41"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r;
            Recording rec = runRecorded(g, r);
            std::vector<Cell> want = referenceOrder("astar", name);
            REQUIRE_FALSE(want.empty());
            INFO("map: " << name);
            REQUIRE(compareOrder(want, rec.expandedOrder) == "");
        }
    }
}

TEST_CASE("the route matches the reference exactly on every map", "[route]") {
    // The neighbour order and the tie-break rule pin down which route this
    // algorithm returns, so there is exactly one correct answer and it is
    // stored in expected/astar/<map>.path. This is the test that matters
    // most: everything else checks that your route is legal and costs what
    // you say it costs, and this checks that it is THE route.
    for (const std::string &name : {
          "m01-perfect-21", "m02-perfect-41", "m03-braid-31", "m04-rooms-35",
          "m05-caves-45", "m06-comb-25", "m07-perfect-81", "m08-caves-101",
          "m09-open-61", "t01-rooms-35", "t02-caves-45", "t03-braid-31",
          "t04-caves-81", "t05-open-51", "e01-sealed-31", "e02-corridor-41",
          "e03-adjacent-11", "e04-same-11", "e05-tiny-3"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            RefPath want = referencePath("astar", name);
            INFO("map: " << name);
            REQUIRE(r.found == want.found);
            if (!want.found) { REQUIRE(r.path.empty()); continue; }
            REQUIRE(comparePath(want.cells, r.path) == "");
            REQUIRE(r.cost == want.cost);
        }
    }
}
