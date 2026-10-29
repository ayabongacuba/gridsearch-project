// tests/test_tsp.cpp -- Stage 6 (and the gate for Stage 7).
//
//   make test-tsp          (links your Floyd and your Dijkstra as well)
//
// Tags: [tiny] the smallest cases   [exact] the true optimum
//       [structure] a well-formed tour   [edge] no tour exists   [e2e] end to end

#include "catch.hpp"

#include <algorithm>
#include <numeric>

#include "grid.h"
#include "harness.h"
#include "matrix.h"

using namespace gs;
using namespace gstest;

// --------------------------------------------------------------------
// A tour is asked for with a Grid, so the hand-built cases are laid out as
// maps whose waypoint table has the property we want to test.
// --------------------------------------------------------------------

static long tourCostFrom(const Matrix &m, const std::vector<int> &order) {
    long total = 0;
    for (size_t i = 0; i + 1 < order.size(); ++i) {
        long leg = m.at(order[i], order[i + 1]);
        if (leg >= INF) return INF;
        total += leg;
    }
    return total;
}

// Independent optimum, by exhaustive search over the oracle's own table.
static long oracleBest(const Grid &g) {
    std::vector<long> d = oracleMatrix(g, g.waypoints);
    const int k = (int)g.waypoints.size();
    if (k < 2) return 0;
    std::vector<int> perm(k - 1);
    std::iota(perm.begin(), perm.end(), 1);
    long best = INF;
    do {
        long c = 0;
        int prev = 0;
        bool dead = false;
        for (int v : perm) {
            long leg = d[(size_t)prev * k + v];
            if (leg >= INF) { dead = true; break; }
            c += leg;
            prev = v;
        }
        if (dead) continue;
        long back = d[(size_t)prev * k + 0];
        if (back >= INF) continue;
        best = std::min(best, c + back);
    } while (std::next_permutation(perm.begin(), perm.end()));
    return best;
}

// =====================================================================
// The smallest cases
// =====================================================================

TEST_CASE("a single stop is a tour of cost zero", "[tiny]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 1}});
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost == 0);
}

TEST_CASE("two stops: out and back", "[tiny]") {
    Grid g = make({"#####",
                   "#...#",
                   "#####"}, {{1, 1}, {1, 3}});
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost == 4);                       // two steps there, two back
    REQUIRE(t.order.front() == 0);
    REQUIRE(t.order.back() == 0);
}

TEST_CASE("three stops in a line", "[tiny]") {
    Grid g = make({"#######",
                   "#.....#",
                   "#######"}, {{1, 1}, {1, 3}, {1, 5}});
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost == oracleBest(g));
    REQUIRE(t.order.size() == 4);               // 0, a, b, 0
}

// =====================================================================
// The true optimum
// =====================================================================

TEST_CASE("it matches the optimum on the shipped maps", "[exact]") {
    for (const std::string &name : {"w01-rooms-31", "w02-caves-45",
                                    "w03-braid-41"}) {
        SECTION(name) {
            Grid g = load(name);
            Tour t = bestTour(g, g.waypoints);
            INFO("map: " << name);
            REQUIRE(t.cost == oracleBest(g));
        }
    }
}

TEST_CASE("an asymmetric table is handled correctly", "[exact]") {
    // Terrain makes going i -> j cost a different amount from j -> i, because
    // cost is charged on entering a cell.
    //
    // Worth knowing, and easy to get wrong in either direction: the table
    // being asymmetric does NOT mean a tour costs a different amount walked
    // backwards. d(i,j) is the total cost of the cells on the cheapest route
    // between them, including both ends, minus cost(i). Around a closed tour
    // those cost(i) terms cancel, so forward and reverse always cost the
    // same. Halving the permutations on that basis is safe here -- though it
    // would not be on a graph whose asymmetry came from somewhere else, such
    // as a one-way street.
    Grid g = make({"#########",
                   "#.9.1.9.#",
                   "#.......#",
                   "#9.1.9.1#",
                   "#.......#",
                   "#########"}, {{1, 1}, {1, 3}, {3, 5}, {1, 7}, {3, 1}});
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost == oracleBest(g));
}

TEST_CASE("going to the nearest unvisited stop is not good enough", "[exact]") {
    // An open room with six stops placed so that greed strands one of them
    // and has to pay for a long crossing at the end. Nearest-neighbour costs
    // 50% more than the optimum here. If your Stage 6 submission is a
    // heuristic rather than exhaustive search, this is where it shows.
    std::vector<std::string> rows;
    for (int r = 0; r < 15; ++r) {
        std::string row;
        for (int c = 0; c < 15; ++c)
            row += (r == 0 || c == 0 || r == 14 || c == 14) ? '#' : '.';
        rows.push_back(row);
    }
    Grid g = make(rows, {{6, 10}, {9, 7}, {5, 6}, {2, 10}, {9, 12}, {1, 4}});

    Tour t = bestTour(g, g.waypoints);
    long best = oracleBest(g);
    Matrix m = waypointMatrix(g, g.waypoints, false);
    long greedy = nearestNeighbourCost(m);

    INFO("optimal " << best << ", nearest-neighbour " << greedy);
    REQUIRE(greedy > best);                    // the trap really is a trap
    REQUIRE(greedy >= best * 3 / 2);           // and by a wide margin
    REQUIRE(t.cost == best);
}

// =====================================================================
// A well-formed tour
// =====================================================================

TEST_CASE("the tour starts and ends at the depot", "[structure]") {
    Grid g = load("w02-caves-45");
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.order.size() == g.waypoints.size() + 1);
    REQUIRE(t.order.front() == 0);
    REQUIRE(t.order.back() == 0);
}

TEST_CASE("every stop is visited exactly once", "[structure]") {
    Grid g = load("w03-braid-41");
    Tour t = bestTour(g, g.waypoints);
    std::vector<int> seen(g.waypoints.size(), 0);
    for (size_t i = 0; i + 1 < t.order.size(); ++i) seen[t.order[i]]++;
    for (size_t i = 0; i < seen.size(); ++i) {
        INFO("stop " << i << " visited " << seen[i] << " times");
        REQUIRE(seen[i] == 1);
    }
}

TEST_CASE("the cost reported is the cost of the order printed", "[structure]") {
    Grid g = load("w02-caves-45");
    Tour t = bestTour(g, g.waypoints);
    Matrix m = waypointMatrix(g, g.waypoints, false);
    REQUIRE(t.cost == tourCostFrom(m, t.order));
}

TEST_CASE("the tour matches the reference exactly", "[exact]") {
    // Several tours can share the cheapest cost, so which one you return is
    // decided by HOW you enumerate. Generate the orderings with
    // std::next_permutation starting from [1, 2, ..., K-1] and replace your
    // best only when a candidate is strictly cheaper, and you get the
    // lexicographically first cheapest tour -- which is what is stored.
    struct Case { const char *map; const char *order; };
    const Case cases[] = {
        {"w01-rooms-31", "0 1 4 2 5 3 0"},
        {"w02-caves-45", "0 4 7 3 1 5 2 6 0"},
        {"w03-braid-41", "0 3 6 1 4 2 7 5 0"},
    };
    for (const Case &c : cases) {
        SECTION(c.map) {
            Grid g = load(c.map);
            Tour t = bestTour(g, g.waypoints);
            std::string got;
            for (int v : t.order) { if (!got.empty()) got += ' '; got += std::to_string(v); }
            INFO("map: " << c.map);
            REQUIRE(got == c.order);
        }
    }
}

TEST_CASE("the number of orderings examined is reported", "[structure]") {
    // Not audited -- pruning is allowed and encouraged. It just has to be
    // a real number.
    Grid g = load("w01-rooms-31");
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.candidates >= 1);
}

// =====================================================================
// No tour exists
// =====================================================================

TEST_CASE("a stop that cannot be reached means there is no tour", "[edge]") {
    Grid g = make({"#########",
                   "#...#...#",
                   "#...#...#",
                   "#########"}, {{1, 1}, {2, 2}, {1, 7}});
    Tour t = bestTour(g, g.waypoints);
    INFO("stop 2 is sealed in the other room, so no closed tour exists");
    REQUIRE(t.cost >= INF);
}

TEST_CASE("INF does not become a cheap tour by accident", "[edge]") {
    Grid g = make({"#########",
                   "#...#...#",
                   "#...#...#",
                   "#########"}, {{1, 1}, {2, 2}, {1, 7}});
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost >= 0);
}

// =====================================================================
// End to end
// =====================================================================

TEST_CASE("the whole pipeline agrees with the oracle", "[e2e]") {
    Grid g = load("w02-caves-45");
    Matrix m = waypointMatrix(g, g.waypoints, false);
    std::vector<long> truth = oracleMatrix(g, g.waypoints);
    for (int i = 0; i < m.k; ++i)
        for (int j = 0; j < m.k; ++j)
            REQUIRE(m.at(i, j) == truth[(size_t)i * m.k + j]);
    Tour t = bestTour(g, g.waypoints);
    REQUIRE(t.cost == oracleBest(g));
}
