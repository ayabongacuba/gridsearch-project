// tests/test_util.cpp -- Stage 0.
//
//   make test-util
//
// Tags: [parse] reading a map   [cost] what a cell costs
//       [nbr] neighbours and order   [matrix] the K x K table   [render] arrows

#include "catch.hpp"

#include <sstream>

#include "grid.h"
#include "harness.h"

using namespace gs;
using namespace gstest;

TEST_CASE("a map is read back exactly as written", "[parse]") {
    std::istringstream in("3 5\n#####\n#.3.#\n#####\n2\n1 1\n1 3\n");
    Grid g = readGrid(in);
    REQUIRE(g.rows == 3);
    REQUIRE(g.cols == 5);
    REQUIRE(g.map[1] == "#.3.#");
    REQUIRE(g.waypoints.size() == 2);
    REQUIRE(g.start() == Cell{1, 1});
    REQUIRE(g.goal() == Cell{1, 3});
}

TEST_CASE("more than two stops are all kept", "[parse]") {
    std::istringstream in("3 5\n#####\n#...#\n#####\n3\n1 1\n1 2\n1 3\n");
    Grid g = readGrid(in);
    REQUIRE(g.waypoints.size() == 3);
    REQUIRE(g.waypoints[2] == Cell{1, 3});
}

TEST_CASE("open ground costs 1 and digits cost their value", "[cost]") {
    Grid g = make({"#####",
                   "#.39#",
                   "#####"}, {{1, 1}, {1, 3}});
    REQUIRE(g.cost(1, 1) == 1);
    REQUIRE(g.cost(1, 2) == 3);
    REQUIRE(g.cost(1, 3) == 9);
}

TEST_CASE("buildings are not passable open ground is", "[cost]") {
    Grid g = make({"###",
                   "#.#",
                   "###"}, {{1, 1}, {1, 1}});
    REQUIRE(g.passable(1, 1));
    REQUIRE_FALSE(g.passable(0, 0));
    REQUIRE_FALSE(g.passable(-1, 1));      // off the map counts as impassable
    REQUIRE_FALSE(g.passable(3, 1));
}



TEST_CASE("the cost table starts empty with a zero diagonal", "[matrix]") {
    std::vector<long> t = makeCostTable(4);
    REQUIRE(t.size() == 16);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            INFO("entry (" << i << ", " << j << ")");
            if (i == j) REQUIRE(t[(size_t)i * 4 + j] == 0);
            else        REQUIRE(t[(size_t)i * 4 + j] == INF);
        }
}

TEST_CASE("the cost table is indexed row-major", "[matrix]") {
    std::vector<long> t = makeCostTable(3);
    t[(size_t)1 * 3 + 2] = 42;          // entry (1, 2)
    REQUIRE(t[5] == 42);
    REQUIRE(t[(size_t)2 * 3 + 1] == INF);   // (2, 1) is a different entry
}

TEST_CASE("INF is large enough to add two of them together", "[matrix]") {
    // If INF is LONG_MAX, then INF + something wraps round to a negative
    // number and a route through nowhere looks like the cheapest one.
    REQUIRE(INF > 0);
    REQUIRE(INF + INF > 0);
}

TEST_CASE("index and at are inverses of each other", "[nbr]") {
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#####"}, {{1, 1}, {2, 3}});
    for (int r = 0; r < g.rows; ++r)
        for (int c = 0; c < g.cols; ++c) {
            INFO("cell " << r << ", " << c);
            REQUIRE(g.at(g.index(r, c)) == Cell{r, c});
        }
}

TEST_CASE("neighbours() returns walkable cells in N E S W order", "[nbr]") {
    Grid g = make({"#####",
                   "#...#",
                   "#...#",
                   "#...#",
                   "#####"}, {{2, 2}, {1, 1}});
    std::vector<Cell> n = g.neighbours(Cell{2, 2});
    REQUIRE(n.size() == 4);
    REQUIRE(n[0] == Cell{1, 2});
    REQUIRE(n[1] == Cell{2, 3});
    REQUIRE(n[2] == Cell{3, 2});
    REQUIRE(n[3] == Cell{2, 1});
}

TEST_CASE("neighbours() skips buildings and the edge of the map", "[nbr]") {
    Grid g = make({"###",
                   "#..",
                   "#.."}, {{1, 1}, {2, 2}});
    std::vector<Cell> n = g.neighbours(Cell{1, 1});
    REQUIRE(n.size() == 2);
    REQUIRE(n[0] == Cell{1, 2});        // east before south
    REQUIRE(n[1] == Cell{2, 1});
}

TEST_CASE("a route renders as arrows in the direction of travel", "[render]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 3}});
    std::vector<Cell> path = {{1, 1}, {1, 2}, {1, 3}};
    std::vector<std::string> out = renderPath(g, path);
    REQUIRE(out[1][1] == 'S');
    REQUIRE(out[1][2] == '>');
    REQUIRE(out[1][3] == 'G');
}

TEST_CASE("vertical travel renders as up and down arrows", "[render]") {
    Grid g = make({"###",
                   "#.#",
                   "#.#",
                   "#.#",
                   "###"}, {{3, 1}, {1, 1}});
    std::vector<Cell> path = {{3, 1}, {2, 1}, {1, 1}};
    std::vector<std::string> out = renderPath(g, path);
    REQUIRE(out[3][1] == 'S');
    REQUIRE(out[2][1] == '^');
    REQUIRE(out[1][1] == 'G');
}

TEST_CASE("validatePath accepts a good route and names the fault in a bad one",
          "[render]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 3}});
    REQUIRE(validatePath(g, {{1, 1}, {1, 2}, {1, 3}}, 2) == "");
    REQUIRE(validatePath(g, {{1, 1}, {1, 3}}, 1) != "");          // not adjacent
    REQUIRE(validatePath(g, {{1, 1}, {1, 2}, {1, 3}}, 99) != "");  // wrong cost
    REQUIRE(validatePath(g, {{1, 2}, {1, 3}}, 1) != "");            // wrong start
}
