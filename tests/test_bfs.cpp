// tests/test_bfs.cpp -- Stage 2.
//
//   make test-bfs
//
// Tags: [route] legality   [cost] arithmetic   [optimal] fewest steps
//       [frontier] how much it holds at once   [edge] awkward maps
//       [trace] the recording   [order] exact expansion sequence

#include "catch.hpp"

#include "grid.h"
#include "harness.h"
#include "search.h"

using namespace gs;
using namespace gstest;

// =====================================================================
// Everything Stage 1 required is still required
// =====================================================================

TEST_CASE("the route it returns is a legal walk", "[route]") {
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45", "m09-open-61"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            INFO("map: " << name);
            REQUIRE(validatePath(g, r.path, r.cost) == "");
        }
    }
}

TEST_CASE("the reported cost equals the cost of the route printed", "[cost]") {
    Grid g = load("m05-caves-45");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    long sum = 0;
    for (size_t i = 1; i < r.path.size(); ++i)
        sum += g.cost(r.path[i].r, r.path[i].c);
    REQUIRE(r.cost == sum);
}

// =====================================================================
// What makes it breadth-first: it must be optimal in steps
// =====================================================================

TEST_CASE("it returns the fewest possible steps", "[optimal]") {
    for (const std::string &name : {"m01-perfect-21", "m03-braid-31",
                                    "m04-rooms-35", "m05-caves-45",
                                    "m06-comb-25", "m09-open-61"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r = search(g, g.start(), g.goal());
            REQUIRE(r.found);
            long best = fewestSteps(g, g.start(), g.goal());
            INFO("map: " << name);
            REQUIRE((long)r.path.size() - 1 == best);
        }
    }
}

TEST_CASE("where two routes tie either is fine but the count is not",
          "[optimal]") {
    // An open room: many 4-step routes from one corner to the other.
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{1, 1}, {3, 3}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() - 1 == 4);
    REQUIRE(validatePath(g, r.path, r.cost) == "");
}

TEST_CASE("on a map where the cheap route is longer it still takes the short one",
          "[optimal]") {
    // Stage 2 does not care about cost. Going straight through the 9s is
    // 2 steps; going around is 4. Breadth-first must return 2.
    Grid g = make({"#####",
                   "#.9.#",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 3}});
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(r.path.size() - 1 == 2);
    REQUIRE(r.cost == 10);
}

// =====================================================================
// The frontier -- where the classic bug shows up
// =====================================================================

TEST_CASE("the frontier stays a ring not a flood", "[frontier]") {
    // On an open field a correct breadth-first frontier is O(sqrt(N)).
    // Marking cells as seen only when you pop them makes it O(N) instead,
    // while still returning the correct route -- so only this catches it.
    Grid g = load("m09-open-61");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    long open = 0;
    for (const std::string &row : g.map)
        for (char ch : row) if (ch != '#') open++;
    INFO("open cells: " << open << ", your max frontier: " << r.maxFrontier);
    REQUIRE(r.maxFrontier < open / 4);
}

TEST_CASE("no cell is ever queued twice", "[frontier]") {
    Grid g = load("m04-rooms-35");
    Result r;
    Recording rec = runRecorded(g, r);
    std::vector<int> pushes(g.rows * g.cols, 0);
    for (const Event &e : rec.events)
        if (e.kind == 'P') pushes[g.index(e.r, e.c)]++;
    for (int n = 0; n < g.rows * g.cols; ++n) {
        INFO("cell " << (n / g.cols) << "," << (n % g.cols)
             << " joined the frontier " << pushes[n] << " times");
        REQUIRE(pushes[n] <= 1);
    }
}

TEST_CASE("nothing is discarded because nothing is queued twice", "[frontier]") {
    Grid g = load("m04-rooms-35");
    Result r;
    Recording rec = runRecorded(g, r);
    long discards = 0;
    for (const Event &e : rec.events) if (e.kind == 'D') discards++;
    REQUIRE(discards == 0);
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
}

TEST_CASE("a long corridor is handled without difficulty", "[edge]") {
    Grid g = load("e02-corridor-41");
    Result r = search(g, g.start(), g.goal());
    REQUIRE(r.found);
    REQUIRE(validatePath(g, r.path, r.cost) == "");
    REQUIRE(r.maxFrontier < 10);          // a corridor has nowhere to branch
}

// =====================================================================
// The recording
// =====================================================================

TEST_CASE("the recording is internally consistent", "[trace]") {
    for (const std::string &name : {"m01-perfect-21", "m04-rooms-35",
                                    "m05-caves-45", "m09-open-61"}) {
        SECTION(name) {
            Grid g = load(name);
            Result r;
            Recording rec = runRecorded(g, r);
            INFO("map: " << name);
            REQUIRE(checkRecording(g, r, rec) == "");
        }
    }
}

TEST_CASE("the start is committed to first and has no parent", "[trace]") {
    Grid g = load("m01-perfect-21");
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE_FALSE(rec.expandedOrder.empty());
    REQUIRE(rec.expandedOrder[0] == g.start());
    const Event *first = nullptr;
    for (const Event &e : rec.events) if (e.kind == 'E') { first = &e; break; }
    REQUIRE(first->pr == -1);
}

// =====================================================================
// The exact expansion sequence
// =====================================================================

TEST_CASE("cells are committed to in rings of increasing distance", "[order]") {
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{2, 2}, {1, 1}});
    Result r;
    Recording rec = runRecorded(g, r);
    // The centre, then its four neighbours in N,E,S,W order, then the corners.
    REQUIRE(rec.expandedOrder.size() >= 5);
    REQUIRE(rec.expandedOrder[0] == Cell{2, 2});
    REQUIRE(rec.expandedOrder[1] == Cell{1, 2});   // north
    REQUIRE(rec.expandedOrder[2] == Cell{2, 3});   // east
    REQUIRE(rec.expandedOrder[3] == Cell{3, 2});   // south
    REQUIRE(rec.expandedOrder[4] == Cell{2, 1});   // west
}

TEST_CASE("the expansion sequence matches the reference exactly", "[order]") {
    Grid g = make({"#####",
                   "#...#",
                   "#.#.#",
                   "#...#",
                   "#####"}, {{1, 2}, {3, 2}});
    Result r;
    Recording rec = runRecorded(g, r);
    REQUIRE(sequence(rec) == "1,2 1,3 1,1 2,3 2,1 3,3 3,1 3,2");
}

TEST_CASE("the full commit order matches the reference on every map", "[order]") {
    // The small hand-built rooms above check the rules in isolation. This
    // checks that they hold across a whole run on every shipped map: the
    // cells you commit to, in the order you commit to them, compared one by
    // one against expected/bfs/<map>.seq. A rule that is only wrong in
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
            std::vector<Cell> want = referenceOrder("bfs", name);
            REQUIRE_FALSE(want.empty());
            INFO("map: " << name);
            REQUIRE(compareOrder(want, rec.expandedOrder) == "");
        }
    }
}

TEST_CASE("the route matches the reference exactly on every map", "[route]") {
    // The neighbour order and the tie-break rule pin down which route this
    // algorithm returns, so there is exactly one correct answer and it is
    // stored in expected/bfs/<map>.path. This is the test that matters
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
            RefPath want = referencePath("bfs", name);
            INFO("map: " << name);
            REQUIRE(r.found == want.found);
            if (!want.found) { REQUIRE(r.path.empty()); continue; }
            REQUIRE(comparePath(want.cells, r.path) == "");
            REQUIRE(r.cost == want.cost);
        }
    }
}
