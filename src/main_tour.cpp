// main_tour.cpp -- driver for stage 6. Provided.
//
// Builds the waypoint matrix with your stage 5 code, hands it to your
// bestTour(), then reconstructs the actual cell path of each leg so the
// tour can be drawn.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "grid.h"
#include "matrix.h"
#include "search.h"
#include "viz.h"

using namespace gs;

int main(int argc, char **argv) {
    bool wantViz = false, quiet = false;
    std::string gridPath;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--viz") wantViz = true;
        else if (a == "--quiet") quiet = true;
        else if (!a.empty() && a[0] != '-') gridPath = a;
    }
    Grid g = gridPath.empty() ? readGrid(std::cin) : readGridFile(gridPath);
    std::string inst = gridPath;
    if (size_t s = inst.find_last_of("/\\"); s != std::string::npos) inst = inst.substr(s + 1);
    if (size_t d = inst.rfind('.'); d != std::string::npos) inst = inst.substr(0, d);
    if (inst.empty()) inst = "stdin";

    auto tm0 = std::chrono::steady_clock::now();
    Tour tour = bestTour(g, g.waypoints);
    auto tm1 = std::chrono::steady_clock::now();
    // Rebuilt here only so the driver can check your tour and draw it.
    Matrix m = waypointMatrix(g, g.waypoints, false);
    auto tm2 = std::chrono::steady_clock::now();

    auto ms = [](auto a, auto b) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    };

    std::cout << "METHOD " << TOUR_STAGE << '\n'
              << "K " << m.k << '\n'
              << "COST " << (tour.cost >= INF ? -1 : tour.cost) << '\n'
              << "CANDIDATES " << tour.candidates << '\n'
              << "TOUR_MS " << ms(tm0, tm1) << '\n'
              << "TOUR_US " << std::chrono::duration_cast<std::chrono::microseconds>(tm1 - tm0).count() << '\n'
              << "MATRIX_MS " << ms(tm1, tm2) << '\n'
              << "ORDER";
    for (int v : tour.order) std::cout << ' ' << v;
    std::cout << '\n';

    if (!quiet && tour.cost < INF) {
        long check = 0;
        for (size_t i = 0; i + 1 < tour.order.size(); ++i)
            check += m.at(tour.order[i], tour.order[i + 1]);
        std::cerr << (check == tour.cost
            ? "check: reported cost matches the matrix\n"
            : "check: reported cost does NOT match the matrix sum\n");
        std::vector<int> seen(m.k, 0);
        for (size_t i = 0; i + 1 < tour.order.size(); ++i) seen[tour.order[i]]++;
        bool all = true;
        for (int s : seen) if (s != 1) all = false;
        std::cerr << (all ? "check: every waypoint visited exactly once\n"
                          : "check: the tour does not visit each waypoint exactly once\n");
    }

    if (wantViz && tour.cost < INF) {
        std::ostringstream t;
        t << "ALGORITHM " << TOUR_STAGE << "\nINSTANCE " << inst << '\n'
          << "GRID " << g.rows << ' ' << g.cols << "\nMAP\n";
        for (const std::string &row : g.map) t << row << '\n';
        t << "WAYPOINTS " << g.waypoints.size() << '\n';
        for (const Cell &w : g.waypoints) t << "X " << w.r << ' ' << w.c << '\n';
        t << "STATS " << ms(tm1, tm2) << ' ' << ms(tm0, tm1) << ' ' << tour.candidates << '\n';
        t << "MATRIX\n";
        for (int i = 0; i < m.k; ++i) {
            for (int j = 0; j < m.k; ++j) { if (j) t << ' '; t << m.at(i, j); }
            t << '\n';
        }
        t << "TOUR " << tour.cost << "\nORDER";
        for (int v : tour.order) t << ' ' << v;
        t << "\nSEGMENTS\n";
        for (size_t i = 0; i + 1 < tour.order.size(); ++i) {
            int a = tour.order[i], b = tour.order[i + 1];
            Result r = search(g, g.waypoints[a], g.waypoints[b]);
            t << "S " << a << ' ' << b << ' ' << r.path.size() << '\n';
            for (const Cell &p : r.path) t << "W " << p.r << ' ' << p.c << '\n';
        }
        viz::launchTemplate("tour.html", t.str(), std::string("tsp-") + inst);
    }
    return 0;
}
