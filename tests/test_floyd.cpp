// tests/test_floyd.cpp -- Stage 5.
//
//   make test-floyd          (links your Dijkstra as well)
//
// Tags: [agree] the two methods must match   [correct] against an oracle
//       [fw] Floyd-Warshall specifics   [mem] what you allocated

#include "catch.hpp"

#include "grid.h"
#include "harness.h"
#include "matrix.h"

using namespace gs;
using namespace gstest;

// =====================================================================
// The headline test: two exact methods cannot disagree
// =====================================================================

TEST_CASE("Floyd-Warshall and repeated Dijkstra produce the same table",
          "[agree]") {
    for (const std::string &name : {"w01-rooms-31", "w02-caves-45",
                                    "w03-braid-41"}) {
        SECTION(name) {
            Grid g = load(name);
            Matrix fw = waypointMatrix(g, g.waypoints, true);
            Matrix dj = waypointMatrix(g, g.waypoints, false);
            REQUIRE(fw.k == dj.k);
            for (int i = 0; i < fw.k; ++i)
                for (int j = 0; j < fw.k; ++j) {
                    INFO("map " << name << ", entry [" << i << "][" << j << "]");
                    REQUIRE(fw.at(i, j) == dj.at(i, j));
                }
        }
    }
}

// =====================================================================
// Both must also be right
// =====================================================================

TEST_CASE("both methods agree with an independent oracle", "[correct]") {
    for (const std::string &name : {"w01-rooms-31", "w02-caves-45"}) {
        SECTION(name) {
            Grid g = load(name);
            std::vector<long> truth = oracleMatrix(g, g.waypoints);
            for (bool useFloyd : {true, false}) {
                Matrix m = waypointMatrix(g, g.waypoints, useFloyd);
                for (int i = 0; i < m.k; ++i)
                    for (int j = 0; j < m.k; ++j) {
                        INFO((useFloyd ? "FW" : "DIJ") << " entry ["
                             << i << "][" << j << "]");
                        REQUIRE(m.at(i, j) == truth[(size_t)i * m.k + j]);
                    }
            }
        }
    }
}

TEST_CASE("the diagonal is zero", "[correct]") {
    Grid g = load("w01-rooms-31");
    for (bool useFloyd : {true, false}) {
        Matrix m = waypointMatrix(g, g.waypoints, useFloyd);
        for (int i = 0; i < m.k; ++i) REQUIRE(m.at(i, i) == 0);
    }
}

TEST_CASE("the table is not symmetric and differs by exactly the end costs",
          "[correct]") {
    // Cost is charged on entering a cell, so going i -> j pays for j but not
    // for i. The difference between the two directions is therefore exactly
    // cost(j) - cost(i). An implementation that assumes symmetry fails here.
    Grid g = load("w02-caves-45");
    Matrix m = waypointMatrix(g, g.waypoints, false);
    for (int i = 0; i < m.k; ++i)
        for (int j = 0; j < m.k; ++j) {
            if (m.at(i, j) >= INF || m.at(j, i) >= INF) continue;
            long lhs = m.at(i, j) - m.at(j, i);
            long rhs = g.cost(g.waypoints[j].r, g.waypoints[j].c)
                     - g.cost(g.waypoints[i].r, g.waypoints[i].c);
            INFO("entry [" << i << "][" << j << "]");
            REQUIRE(lhs == rhs);
        }
}

TEST_CASE("an unreachable stop is INF in both methods", "[correct]") {
    //   . . . # . . .     two rooms, no door between them
    Grid g = make({"#########",
                   "#...#...#",
                   "#...#...#",
                   "#########"}, {{1, 1}, {1, 7}, {2, 2}});
    for (bool useFloyd : {true, false}) {
        Matrix m = waypointMatrix(g, g.waypoints, useFloyd);
        INFO((useFloyd ? "Floyd-Warshall" : "repeated Dijkstra"));
        REQUIRE(m.at(0, 1) >= INF);
        REQUIRE(m.at(1, 0) >= INF);
        REQUIRE(m.at(0, 2) < INF);       // these two are in the same room
    }
}

TEST_CASE("INF does not leak into a reachable entry", "[correct]") {
    // The classic arithmetic slip: INF + something wrapping around and
    // becoming a small number.
    Grid g = make({"#########",
                   "#...#...#",
                   "#...#...#",
                   "#########"}, {{1, 1}, {1, 7}, {2, 2}});
    Matrix m = waypointMatrix(g, g.waypoints, true);
    for (int i = 0; i < m.k; ++i)
        for (int j = 0; j < m.k; ++j)
            REQUIRE(m.at(i, j) >= 0);
}

// =====================================================================
// Floyd-Warshall specifics
// =====================================================================

TEST_CASE("a route needing two intermediate cells is still found", "[fw]") {
    // A single pass of relaxation finds routes with one intermediate. This
    // map needs several, so it catches an implementation that does not
    // iterate over every k.
    Grid g = make({"#######",
                   "#.....#",
                   "#####.#",
                   "#.....#",
                   "#.#####",
                   "#.....#",
                   "#######"}, {{1, 1}, {5, 5}});
    Matrix m = waypointMatrix(g, g.waypoints, true);
    long truth = cheapestCost(g, g.waypoints[0], g.waypoints[1]);
    REQUIRE(truth > 0);
    REQUIRE(m.at(0, 1) == truth);
}

TEST_CASE("the intermediate-cell loop is the outermost one", "[fw]") {
    // If the k loop is nested inside the i and j loops, the table is filled
    // in the wrong order and some routes are never composed. On a long
    // winding corridor the wrong nesting produces a cost that is too large.
    Grid g = make({"###########",
                   "#.........#",
                   "#########.#",
                   "#.........#",
                   "#.#########",
                   "#.........#",
                   "###########"}, {{1, 1}, {5, 9}});
    Matrix fw = waypointMatrix(g, g.waypoints, true);
    Matrix dj = waypointMatrix(g, g.waypoints, false);
    INFO("if these differ, check which of your three loops is outermost");
    REQUIRE(fw.at(0, 1) == dj.at(0, 1));
    REQUIRE(fw.at(0, 1) == cheapestCost(g, g.waypoints[0], g.waypoints[1]));
}

TEST_CASE("terrain costs are respected not just connectivity", "[fw]") {
    Grid g = make({"#######",
                   "#.999.#",
                   "#.....#",
                   "#######"}, {{1, 1}, {1, 5}});
    Matrix fw = waypointMatrix(g, g.waypoints, true);
    REQUIRE(fw.at(0, 1) == 6);      // around the bottom, not through the 9s
}

// =====================================================================
// What you allocated
// =====================================================================

TEST_CASE("Floyd-Warshall reports a table of at least V squared", "[mem]") {
    Grid g = load("w01-rooms-31");
    Matrix fw = waypointMatrix(g, g.waypoints, true);
    const long V = (long)g.rows * g.cols;
    INFO("V = " << V << ", so the table is at least " << (V * V * 8)
         << " bytes; you reported " << fw.peakBytes);
    REQUIRE(fw.peakBytes >= V * V * 8);
}

TEST_CASE("repeated Dijkstra reports far less than Floyd-Warshall", "[mem]") {
    Grid g = load("w01-rooms-31");
    Matrix fw = waypointMatrix(g, g.waypoints, true);
    Matrix dj = waypointMatrix(g, g.waypoints, false);
    INFO("FW: " << fw.peakBytes << " bytes, Dijkstra: " << dj.peakBytes);
    REQUIRE(dj.peakBytes < fw.peakBytes / 100);
}

TEST_CASE("both report doing some work", "[mem]") {
    Grid g = load("w01-rooms-31");
    REQUIRE(waypointMatrix(g, g.waypoints, true).relaxations > 0);
    REQUIRE(waypointMatrix(g, g.waypoints, false).relaxations > 0);
}
