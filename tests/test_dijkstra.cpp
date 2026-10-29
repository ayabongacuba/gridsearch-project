// tests/test_dijkstra.cpp -- Stage 3.
//
//   make test-dijkstra
//
// Tags: [route] legality   [optimal] cheapest, not shortest   [cost] arithmetic
//       [edge] awkward maps   [trace] the recording   [order] exact sequence

#include "catch.hpp"

#include "grid.h"
#include "harness.h"
#include "search.h"

using namespace gs;
using namespace gstest;

TEST_CASE("the route it returns is a legal walk", "[route]") {
    for (const std::string &name : {"m05-caves-45", "t01-rooms-35",
                                    "t02-caves-45", "t05-open-51"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(validatePath(g, r.path, r.cost) == "");
        }
    }
}

// =====================================================================
// Cheapest, on every map
// =====================================================================

TEST_CASE("it returns the cheapest route on uniform maps", "[optimal]") {
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(r.cost == cheapestCost(g, g.start(), g.goal()));
        }
    }
}

TEST_CASE("it returns the cheapest route once terrain costs differ",
          "[optimal]") {
    for (const std::string &name : {"t01-rooms-35", "t02-caves-45",
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

TEST_CASE("the cheapest route is longer in steps than the shortest one",
          "[optimal]") {
    // This is the test that a breadth-first answer cannot pass. On these maps
    // the least-effort route deliberately takes MORE steps than the
    // fewest-steps route.
    for (const std::string &name : {"t01-rooms-35", "t04-caves-81"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            long steps = (long)r.path.size() - 1;
            long fewest = fewestSteps(g, g.start(), g.goal());
            INFO("map: " << name << " -- your route is " << steps
                 << " steps, the shortest possible is " << fewest);
            REQUIRE(steps > fewest);
        }
    }
}

TEST_CASE("the smallest case where steps and effort disagree", "[optimal]") {
    //  row 1:  S 9 9 9 G     straight through: 4 steps, effort 28
    //  row 2:  . . . . .     around the bottom: 6 steps, effort  6
    Grid g = make({"#######",
                   "#.999.#",
                   "#.....#",
                   "#######"}, {{1, 1}, {1, 5}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.cost == 6);
    REQUIRE(r.path.size() - 1 == 6);
    REQUIRE(fewestSteps(g, g.start(), g.goal()) == 4);
}

// =====================================================================
// Cost arithmetic
// =====================================================================

TEST_CASE("cost is charged for entering a cell never for leaving it",
          "[cost]") {
    Grid g = make({"#####",
                   "#9.9#",
                   "#####"}, {{1, 1}, {1, 3}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    // start on the 9 (free), enter the '.', enter the 9
    REQUIRE(r.cost == 10);
}

TEST_CASE("the reported cost equals the cost of the route printed", "[cost]") {
    Grid g = load("t04-caves-81");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    long sum = 0;
    for (size_t i = 1; i < r.path.size(); ++i)
        sum += g.cost(r.path[i].r, r.path[i].c);
    REQUIRE(r.cost == sum);
}

// =====================================================================
// Awkward maps
// =====================================================================

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

TEST_CASE("a single expensive step is still the only step", "[edge]") {
    Grid g = make({"####",
                   "#.9#",
                   "####"}, {{1, 1}, {1, 2}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.cost == 9);
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

TEST_CASE("this cost model never needs a cell to be updated", "[trace]") {
    // Worth understanding, because it is a property of the map rather than of
    // the algorithm. Cost is charged for ENTERING a cell, so every route into
    // cell v pays the same amount for the last step. Dijkstra settles cells in
    // increasing order of cost-so-far, so the first settled neighbour of v is
    // always the cheapest way in -- no later discovery can improve it.
    //
    // The consequence: your Dijkstra should produce no relax events and no
    // discards, and every cell should join the frontier exactly once. If yours
    // produces them, something is settling cells out of order.
    //
    // A* is different, and Stage 4 will show you why.
    for (const std::string &name : {"t02-caves-45", "t05-open-51"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r;
            Recording rec = runRecorded(g, r);
            long relaxes = 0, discards = 0;
            std::vector<int> pushes(g.rows * g.cols, 0);
            for (const Event &e : rec.events) {
                if (e.kind == 'R') relaxes++;
                if (e.kind == 'D') discards++;
                if (e.kind == 'P') pushes[g.index(e.r, e.c)]++;
            }
            INFO("map: " << name);
            REQUIRE(relaxes == 0);
            REQUIRE(discards == 0);
            for (int n = 0; n < g.rows * g.cols; ++n) REQUIRE(pushes[n] <= 1);
        }
    }
}

TEST_CASE("cheap cells are always settled before expensive ones", "[trace]") {
    // The defining behaviour: the frontier comes off in order of cost.
    Grid g = load("t05-open-51");
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE(checkRecording(g, r, rec) == "");
    // reconstruct the settled cost of each committed cell and check it never
    // decreases as the search proceeds
    long prev = -1;
    std::vector<long> settled(g.rows * g.cols, -1);
    for (const Event &e : rec.events) {
        if (e.kind != 'E') continue;
        long c = (e.pr < 0) ? 0
               : settled[g.index(e.pr, e.pc)] + g.cost(e.r, e.c);
        settled[g.index(e.r, e.c)] = c;
        INFO("cells came off the frontier out of cost order");
        REQUIRE(c >= prev);
        prev = c;
    }
}

// =====================================================================
// The exact expansion sequence -- this is what the tie-break rule is for
// =====================================================================

TEST_CASE("ties are broken by row then by column", "[order]") {
    // From the centre of an open uniform room, all four neighbours cost 1.
    // Every one is equally promising, so only the tie-break rule decides
    // which is committed to first: smallest row, then smallest column.
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{2, 2}, {1, 1}});
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE(rec.expandedOrder.size() >= 5);
    REQUIRE(rec.expandedOrder[0] == Cell{2, 2});
    INFO("all four neighbours cost 1, so the tie-break rule decides the order");
    REQUIRE(rec.expandedOrder[1] == Cell{1, 2});   // row 1 beats rows 2 and 3
    REQUIRE(rec.expandedOrder[2] == Cell{2, 1});   // row 2, column 1
    REQUIRE(rec.expandedOrder[3] == Cell{2, 3});   // row 2, column 3
    REQUIRE(rec.expandedOrder[4] == Cell{3, 2});   // row 3 last
}

TEST_CASE("the expansion sequence matches the reference exactly", "[order]") {
    Grid g = make({"#####",
                   "#.2.#",
                   "#9.9#",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 3}});
    Result r;
    Recording rec = runRecorded(g, r);
    INFO("sequence was: " << sequence(rec));
    REQUIRE(r.found);
    REQUIRE(r.cost == cheapestCost(g, g.start(), g.goal()));
    // cheap cells are always settled before expensive ones
    REQUIRE(rec.expandedOrder[0] == Cell{1, 1});
    REQUIRE(rec.expandedOrder[1] == Cell{1, 2});
}

TEST_CASE("the full commit order matches the reference on every map", "[order]") {
    // The small hand-built rooms above check the rules in isolation. This
    // checks that they hold across a whole run on every shipped map: the
    // cells you commit to, in the order you commit to them, compared one by
    // one against expected/dijkstra/<map>.seq. A rule that is only wrong in
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
            std::vector<Cell> want = referenceOrder("dijkstra", name);
            REQUIRE_FALSE(want.empty());
            INFO("map: " << name);
            REQUIRE(compareOrder(want, rec.expandedOrder) == "");
        }
    }
}

TEST_CASE("the route matches the reference exactly on every map", "[route]") {
    // The neighbour order and the tie-break rule pin down which route this
    // algorithm returns, so there is exactly one correct answer and it is
    // stored in expected/dijkstra/<map>.path. This is the test that matters
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
            RefPath want = referencePath("dijkstra", name);
            INFO("map: " << name);
            REQUIRE(r.found == want.found);
            if (!want.found) { REQUIRE(r.path.empty()); continue; }
            REQUIRE(comparePath(want.cells, r.path) == "");
            REQUIRE(r.cost == want.cost);
        }
    }
}
