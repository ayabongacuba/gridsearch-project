// main_matrix.cpp -- driver for stage 5. Provided.
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "grid.h"
#include "matrix.h"
#include "search.h"
#include "viz.h"

using namespace gs;

int main(int argc, char **argv) {
    bool useFloyd = true, wantViz = false, quiet = false;
    std::string gridPath;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--fw") useFloyd = true;
        else if (a == "--dij") useFloyd = false;
        else if (a == "--viz") wantViz = true;
        else if (a == "--quiet") quiet = true;
        else if (!a.empty() && a[0] != '-') gridPath = a;
    }
    Grid g = gridPath.empty() ? readGrid(std::cin) : readGridFile(gridPath);

    std::string inst = gridPath;
    if (size_t s = inst.find_last_of("/\\"); s != std::string::npos) inst = inst.substr(s + 1);
    if (size_t d = inst.rfind('.'); d != std::string::npos) inst = inst.substr(0, d);
    if (inst.empty()) inst = "stdin";

    auto t0 = std::chrono::steady_clock::now();
    Matrix m = waypointMatrix(g, g.waypoints, useFloyd);
    auto t1 = std::chrono::steady_clock::now();
    long us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    long ms = us / 1000;

    std::cout << "METHOD " << (useFloyd ? "FW" : "DIJ") << '\n'
              << "V " << g.rows * g.cols << '\n'
              << "K " << m.k << '\n'
              << "TIME_MS " << ms << '\n'
              << "TIME_US " << us << '\n'
              << "PEAK_KB " << (m.peakBytes / 1024) << '\n'
              << "RELAXATIONS " << m.relaxations << '\n'
              << "MATRIX\n";
    for (int i = 0; i < m.k; ++i) {
        for (int j = 0; j < m.k; ++j) {
            if (j) std::cout << ' ';
            long v = m.at(i, j);
            if (v >= INF) std::cout << "INF"; else std::cout << v;
        }
        std::cout << '\n';
    }

    if (!quiet) {
        // Cost is charged for ENTERING a cell, so the matrix is NOT symmetric:
        // going i -> j pays for j but not for i. The difference between the two
        // directions is exactly cost(j) - cost(i), and that is what to check.
        bool ok = true;
        for (int i = 0; i < m.k && ok; ++i)
            for (int j = 0; j < m.k; ++j) {
                if (m.at(i, j) >= INF || m.at(j, i) >= INF) continue;
                long lhs = m.at(i, j) - m.at(j, i);
                long rhs = g.cost(g.waypoints[j].r, g.waypoints[j].c)
                         - g.cost(g.waypoints[i].r, g.waypoints[i].c);
                if (lhs != rhs) { ok = false; break; }
            }
        std::cerr << (ok
            ? "check: matrix is consistent (d(i,j) - d(j,i) == cost(j) - cost(i))\n"
            : "check: matrix is inconsistent -- the two directions do not differ by the endpoint costs\n");
    }

    if (wantViz) {
        std::ostringstream t;
        t << "ALGORITHM " << MATRIX_STAGE << " (" << (useFloyd ? "fw" : "dij") << ")\n"
          << "INSTANCE " << inst << "\nGRID " << g.rows << ' ' << g.cols << "\nMAP\n";
        for (const std::string &row : g.map) t << row << '\n';
        t << "WAYPOINTS " << g.waypoints.size() << '\n';
        for (const Cell &w : g.waypoints) t << "X " << w.r << ' ' << w.c << '\n';
        t << "STATS " << ms << ' ' << (m.peakBytes / 1024) << ' ' << m.relaxations << '\n';
        t << "MATRIX\n";
        for (int i = 0; i < m.k; ++i) {
            for (int j = 0; j < m.k; ++j) {
                if (j) t << ' ';
                long v = m.at(i, j);
                if (v >= INF) t << "-1"; else t << v;
            }
            t << '\n';
        }
        viz::launchTemplate("tour.html", t.str(),
                            std::string(MATRIX_STAGE) + "-" + inst);
    }
    return 0;
}
