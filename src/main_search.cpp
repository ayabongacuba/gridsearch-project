// main_search.cpp -- the driver every stage binary shares.
// It reads the grid, calls your search(), prints the stats block, checks
// your path, and optionally opens the visualiser.
//
// Provided. You should not need to change this file.

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <iostream>
#include <string>

#include "grid.h"
#include "search.h"
#include "live.h"
#include "trace.h"
#include "viz.h"
#include "window.h"

using namespace gs;

static void usage(const char *exe) {
    std::cerr <<
        "usage: " << exe << " [options] [grid-file]\n"
        "  reads the grid from the file, or from stdin if none is given\n\n"
        "  --window         replay in a native window (needs SDL2)\n"
        "  --viz            open the replay in a browser when the search finishes\n"
        "  --live           open the browser FIRST and stream the search into it\n"
        "  --pace N         with --live, hold the search to N expansions/second\n"
        "                   (default 1500; use 0 for full speed)\n"
        "  --trace FILE     write the raw event stream to FILE\n"
        "  --render         print the map with the path drawn on it\n"
        "  --quiet          stats only, no path check messages\n";
}

int main(int argc, char **argv) {
    bool wantViz = false, wantLive = false, wantWin = false, render = false, quiet = false;
    double paceRate = 1500;
    std::string tracePath, gridPath;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--viz") wantViz = true;
        else if (a == "--window" || a == "--win") wantWin = true;
        else if (a == "--live") wantLive = true;
        else if (a == "--pace" && i + 1 < argc) paceRate = std::atof(argv[++i]);
        else if (a == "--render") render = true;
        else if (a == "--quiet") quiet = true;
        else if (a == "--trace" && i + 1 < argc) tracePath = argv[++i];
        else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
        else if (!a.empty() && a[0] != '-') gridPath = a;
        else { usage(argv[0]); return 2; }
    }

    Grid g = gridPath.empty() ? readGrid(std::cin) : readGridFile(gridPath);

    std::string instance = gridPath;
    if (size_t s = instance.find_last_of("/\\"); s != std::string::npos)
        instance = instance.substr(s + 1);
    if (size_t d = instance.rfind('.'); d != std::string::npos)
        instance = instance.substr(0, d);
    if (instance.empty()) instance = "stdin";

    if (wantWin && !window::available) {
        std::cerr << "window: this build has no SDL2, falling back to the browser\n";
        wantWin = false; wantViz = true;
    }
    const bool recording = wantViz || wantLive || wantWin || !tracePath.empty();

    if (wantLive) {
        std::string page = viz::slurp(std::string(GS_ROOT) + "/web/player.html");
        int port = page.empty() ? 0 : live::start(page);
        if (!port) {
            std::cerr << "live: could not start the local server, "
                         "falling back to --viz\n";
            wantLive = false; wantViz = true;
        } else {
            trace::sink() = [](const std::string &chunk) { live::server().append(chunk); };
            trace::pace() = paceRate;
            std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
            std::cerr << "live: " << url << "\n";
            viz::openUrl(url);
            // give the browser a moment to connect before the search starts
            for (int w = 0; w < 60 && !live::server().seen.load(); ++w)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    if (recording) trace::begin(g, g.start(), g.goal(), ALGORITHM, instance);

    auto t0 = std::chrono::steady_clock::now();
    Result res = search(g, g.start(), g.goal());
    auto t1 = std::chrono::steady_clock::now();
    long us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    us -= (long)trace::sleptMs() * 1000;   // do not charge the search for pacing
    if (us < 0) us = 0;
    long ms = us / 1000;

    if (recording) trace::end(res.found ? res.path : std::vector<Cell>{});

    if (!res.found) {
        std::cout << "NOPATH\n"
                  << "EXPANDED " << res.expanded << '\n'
                  << "MAXFRONTIER " << res.maxFrontier << '\n'
                  << "TIME_MS " << ms << '\n'
                  << "TIME_US " << us << '\n';
    } else {
        std::cout << "COST " << res.cost << '\n'
                  << "LEN " << res.path.size() << '\n'
                  << "EXPANDED " << res.expanded << '\n'
                  << "MAXFRONTIER " << res.maxFrontier << '\n'
                  << "TIME_MS " << ms << '\n'
                  << "TIME_US " << us << '\n';
        if (render) {
            std::cout << "GRID\n";
            for (const std::string &row : renderPath(g, res.path))
                std::cout << row << '\n';
        }
        if (!quiet) {
            std::string fault = validatePath(g, res.path, res.cost);
            if (fault.empty()) std::cerr << "check: path is legal\n";
            else std::cerr << "check: " << fault << '\n';
        }
    }

    if (!quiet && res.expanded == 0 && res.found)
        std::cerr << "check: you reported EXPANDED 0 -- is your counter wired up?\n";

    if (!tracePath.empty()) {
        std::ofstream f(tracePath);
        f << trace::text();
        std::cerr << "trace: " << tracePath << '\n';
    }
    if (wantWin) {
        if (!window::replay(trace::text(), std::string(ALGORITHM) + " - " + instance)) {
            std::cerr << "window: could not open a window, writing the browser replay instead\n";
            viz::launch(trace::text(), std::string(ALGORITHM) + "-" + instance);
        }
        return 0;
    }
    if (wantLive) {
        live::finish();
        if (!quiet)
            std::cerr << "live: search finished. Close the tab, or wait, to stop serving.\n";
        live::waitForClient(600);
        return 0;
    }
    if (wantViz) {
        if (trace::truncated())
            std::cerr << "viz: event limit hit, the replay is cut short\n";
        viz::launch(trace::text(), std::string(ALGORITHM) + "-" + instance);
    }
    return 0;
}
