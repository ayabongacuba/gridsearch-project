// tests/test_dfs.cpp -- Stage 1.
//
//   make test-dfs        build and run
//   ./bin/test_dfs -s    show every assertion, not just failures
//   ./bin/test_dfs "[route]"     run only the tagged tests
//
// Tags:  [route] legality   [cost] arithmetic   [edge] awkward maps
//        [trace] the recording   [order] exact expansion sequence

#include "catch.hpp"

#include "grid.h"
#include "harness.h"
#include "search.h"

using namespace gs;
using namespace gstest;

// =====================================================================
// Route legality -- true of any correct DFS
// =====================================================================

TEST_CASE("the route it returns is a legal walk", "[route]") {
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45", "m06-comb-25"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(validatePath(g, r.path, r.cost) == "");
        }
    }
}

TEST_CASE("the route starts and ends in the right place", "[route]") {
    Grid g = make({"#####",
                   "#...#",
                   "#.#.#",
                   "#...#",
                   "#####"}, {{1, 1}, {3, 3}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.front() == g.start());
    REQUIRE(r.path.back() == g.goal());
}

TEST_CASE("no step is diagonal and no cell repeats", "[route]") {
    Grid g = load("m05-caves-45");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    std::vector<char> seen(g.rows * g.cols, 0);
    for (size_t i = 0; i < r.path.size(); ++i) {
        REQUIRE(g.passable(r.path[i].r, r.path[i].c));
        REQUIRE_FALSE(seen[g.index(r.path[i])]);
        seen[g.index(r.path[i])] = 1;
        if (i) {
            int d = std::abs(r.path[i].r - r.path[i - 1].r) +
                    std::abs(r.path[i].c - r.path[i - 1].c);
            INFO("step " << i);
            REQUIRE(d == 1);
        }
    }
}

// =====================================================================
// Cost arithmetic -- DFS need not be cheap, but must be honest
// =====================================================================

TEST_CASE("the reported cost equals the cost of the route printed", "[cost]") {
    for (const std::string &name : {"m03-braid-31", "t02-caves-45"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            long sum = 0;
            for (size_t i = 1; i < r.path.size(); ++i)
                sum += g.cost(r.path[i].r, r.path[i].c);
            REQUIRE(r.cost == sum);
        }
    }
}

TEST_CASE("cost counts the cell entered never the cell left", "[cost]") {
    //  S 9 G   -- one step onto the 9, one onto the goal
    Grid g = make({"#####",
                   "#.9.#",
                   "#####"}, {{1, 1}, {1, 3}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 3);
    REQUIRE(r.cost == 10);          // 9 to enter the middle, 1 to enter the goal
}

TEST_CASE("a route is allowed to be far from optimal", "[cost]") {
    // DFS makes no promise about length. This test exists to document that:
    // it must be LEGAL, not short.
    Grid g = load("m05-caves-45");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    long best = fewestSteps(g, g.start(), g.goal());
    REQUIRE(best > 0);
    REQUIRE((long)r.path.size() - 1 >= best);      // never shorter than optimal
}

// =====================================================================
// Awkward maps
// =====================================================================

TEST_CASE("no route exists", "[edge]") {
    Grid g = load("e01-sealed-31");
    Result r = search(g, g.start(), g.goal());
    REQUIRE_FALSE(r.found);
    REQUIRE(r.path.empty());
}

TEST_CASE("the destination is where we already are", "[edge]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 2}, {1, 2}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 1);
    REQUIRE(r.cost == 0);
    REQUIRE(r.expanded == 1);
}

TEST_CASE("the destination is one step away", "[edge]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 2}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 2);
    REQUIRE(r.cost == 1);
}

TEST_CASE("a map with a single open cell", "[edge]") {
    Grid g = make({"###",
                   "#.#",
                   "###"}, {{1, 1}, {1, 1}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() == 1);
}

TEST_CASE("a corridor 798 cells long does not overflow the stack", "[edge]") {
    // If this crashes rather than fails, your DFS is recursive.
    Grid g = load("e02-corridor-41");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(validatePath(g, r.path, r.cost) == "");
}

// =====================================================================
// The recording
// =====================================================================

TEST_CASE("the recording is internally consistent", "[trace]") {
    // m04 matters here: it is the map on which the reference actually
    // discards stale frontier entries, so it is the one that catches a
    // missing trace::discard.
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45", "m06-comb-25"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r;
            Recording rec = runRecorded(g, r);
            INFO("map: " << name);
            REQUIRE(checkRecording(g, r, rec) == "");
        }
    }
}

TEST_CASE("every hook is actually being called", "[trace]") {
    Grid g = load("m04-rooms-35");
    Result r;
    Recording rec = runRecorded(g, r);
    long pushes = 0, expands = 0;
    for (const Event &e : rec.events) {
        if (e.kind == 'P') pushes++;
        if (e.kind == 'E') expands++;
    }
    REQUIRE(expands > 0);
    REQUIRE(pushes >= expands);          // nothing is committed to unseen
    REQUIRE(r.expanded == expands);
    REQUIRE(r.maxFrontier > 0);
}

TEST_CASE("stale frontier entries are reported as discarded", "[trace]") {
    // A cell can join the frontier more than once before it is ever taken
    // off. When the second copy surfaces it must be discarded, and the
    // recording must say so, or the frontier count will not balance.
    Grid g = load("m04-rooms-35");
    Result r;
    Recording rec = runRecorded(g, r);
    long pushes = 0, expands = 0, discards = 0;
    for (const Event &e : rec.events) {
        if (e.kind == 'P') pushes++;
        if (e.kind == 'E') expands++;
        if (e.kind == 'D') discards++;
    }
    INFO("on this map the same cell reaches the frontier more than once, so "
         "some entries must be thrown away when they surface");
    REQUIRE(discards > 0);
    REQUIRE(checkRecording(g, r, rec) == "");
}

TEST_CASE("the start is committed to with no parent", "[trace]") {
    Grid g = load("m01-perfect-21");
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE_FALSE(rec.events.empty());
    const Event *first = nullptr;
    for (const Event &e : rec.events) if (e.kind == 'E') { first = &e; break; }
    REQUIRE(first != nullptr);
    REQUIRE(first->r == g.start().r);
    REQUIRE(first->c == g.start().c);
    REQUIRE(first->pr == -1);
}

// =====================================================================
// The exact expansion sequence
// =====================================================================

TEST_CASE("neighbours are considered N E S W", "[order]") {
    // An open room. From the start every direction is available, so the order
    // in which cells come off the frontier reveals the order they went on.
    //
    //   . . .
    //   . S .        start at the centre of a 3x3 room
    //   . . .
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{2, 2}, {3, 3}});
    Result r;
    Recording rec = runRecorded(g, r);

    // Pushed N,E,S,W onto a stack => popped W first.
    REQUIRE(rec.expandedOrder.size() >= 2);
    REQUIRE(rec.expandedOrder[0] == Cell{2, 2});
    INFO("after the start, a stack-based DFS pushing N,E,S,W must continue west");
    REQUIRE(rec.expandedOrder[1] == Cell{2, 1});
}

TEST_CASE("the expansion sequence matches the reference exactly", "[order]") {
    // A corridor with one junction: correct implementations cannot disagree.
    //
    //   # # # # #
    //   # . . . #      the search must go west from the junction first
    //   # . # . #
    //   # . . . #
    //   # # # # #
    Grid g = make({"#####",
                   "#...#",
                   "#.#.#",
                   "#...#",
                   "#####"}, {{1, 2}, {3, 2}});
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE(sequence(rec) == "1,2 1,1 2,1 3,1 3,2");
}

TEST_CASE("a dead end is backtracked out of not walked through", "[order]") {
    //   # # # # # #
    //   # . . . . #     a spur hangs off to the west; DFS enters it first,
    //   # . # # # #     finds nothing, and comes back
    //   # . # # # #
    //   # # # # # #
    Grid g = make({"######",
                   "#....#",
                   "#.####",
                   "#.####",
                   "######"}, {{1, 1}, {1, 4}});
    Result r;
    Recording rec = runRecorded(g, r);
    // It goes down the spur first (south is pushed after east, so pops first
    // only if west is unavailable) -- the point is that the route returned
    // must not contain the spur, even though the search visited it.
    REQUIRE(r.found);
    REQUIRE(validatePath(g, r.path, r.cost) == "");
    bool visitedSpur = false;
    for (const Cell &p : rec.expandedOrder)
        if (p.r == 3 && p.c == 1) visitedSpur = true;
    REQUIRE(visitedSpur);
    for (const Cell &p : r.path)
        REQUIRE_FALSE((p.r == 3 && p.c == 1));
}

TEST_CASE("the full commit order matches the reference on every map", "[order]") {
    // The small hand-built rooms above check the rules in isolation. This
    // checks that they hold across a whole run on every shipped map: the
    // cells you commit to, in the order you commit to them, compared one by
    // one against expected/dfs/<map>.seq. A rule that is only wrong in
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
            std::vector<Cell> want = referenceOrder("dfs", name);
            REQUIRE_FALSE(want.empty());
            INFO("map: " << name);
            REQUIRE(compareOrder(want, rec.expandedOrder) == "");
        }
    }
}

TEST_CASE("the route matches the reference exactly on every map", "[route]") {
    // The neighbour order and the tie-break rule pin down which route this
    // algorithm returns, so there is exactly one correct answer and it is
    // stored in expected/dfs/<map>.path. This is the test that matters
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
            RefPath want = referencePath("dfs", name);
            INFO("map: " << name);
            REQUIRE(r.found == want.found);
            if (!want.found) { REQUIRE(r.path.empty()); continue; }
            REQUIRE(comparePath(want.cells, r.path) == "");
            REQUIRE(r.cost == want.cost);
        }
    }
}
