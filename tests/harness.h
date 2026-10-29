// tests/harness.h -- helpers shared by every stage's test file.
//
// Nothing here is specific to one algorithm. It builds small maps inline,
// reads back the recording your hooks produced, and checks the things that
// must be true of any correct search.

#ifndef GS_TEST_HARNESS_H
#define GS_TEST_HARNESS_H

#include <algorithm>
#include <deque>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "grid.h"
#include "matrix.h"
#include "search.h"
#include "trace.h"

namespace gstest {

// ---------------------------------------------------------------- maps

// Build a Grid from a picture, without touching the filesystem:
//
//   Grid g = make({"#####",
//                  "#..3#",
//                  "#####"}, {{1,1},{1,3}});
//
inline gs::Grid make(const std::vector<std::string> &rows,
                     const std::vector<gs::Cell> &stops) {
    gs::Grid g;
    g.rows = (int)rows.size();
    g.cols = rows.empty() ? 0 : (int)rows[0].size();
    g.map = rows;
    g.waypoints = stops;
    return g;
}

// Load one of the maps shipped with the project. GS_MAPS is set by the
// Makefile so the tests run from anywhere.
inline gs::Grid load(const std::string &name) {
    return gs::readGridFile(std::string(GS_MAPS) + "/" + name + ".grid");
}

// ---------------------------------------------------------------- oracles

// Fewest steps, computed independently of anything the student wrote.
// Returns -1 if unreachable.
inline long fewestSteps(const gs::Grid &g, gs::Cell a, gs::Cell b) {
    std::vector<long> d(g.rows * g.cols, -1);
    std::deque<int> q;
    d[g.index(a)] = 0;
    q.push_back(g.index(a));
    while (!q.empty()) {
        int n = q.front(); q.pop_front();
        gs::Cell u = g.at(n);
        for (int k = 0; k < 4; ++k) {
            int nr = u.r + gs::DR[k], nc = u.c + gs::DC[k];
            if (!g.passable(nr, nc) || d[g.index(nr, nc)] >= 0) continue;
            d[g.index(nr, nc)] = d[n] + 1;
            q.push_back(g.index(nr, nc));
        }
    }
    return d[g.index(b)];
}

// Cheapest cost, computed independently with a deliberately naive Dijkstra:
// O(V^2) selection, no heap. Written unlike anything a student would submit,
// so a shared misunderstanding cannot hide in both.
inline long cheapestCost(const gs::Grid &g, gs::Cell a, gs::Cell b) {
    const long BIG = std::numeric_limits<long>::max() / 4;
    const int N = g.rows * g.cols;
    std::vector<long> dist(N, BIG);
    std::vector<char> done(N, 0);
    dist[g.index(a)] = 0;
    for (;;) {
        int best = -1;
        for (int n = 0; n < N; ++n)
            if (!done[n] && dist[n] < BIG && (best < 0 || dist[n] < dist[best]))
                best = n;
        if (best < 0) break;
        done[best] = 1;
        gs::Cell u = g.at(best);
        for (int k = 0; k < 4; ++k) {
            int nr = u.r + gs::DR[k], nc = u.c + gs::DC[k];
            if (!g.passable(nr, nc)) continue;
            long nd = dist[best] + g.cost(nr, nc);
            if (nd < dist[g.index(nr, nc)]) dist[g.index(nr, nc)] = nd;
        }
    }
    long d = dist[g.index(b)];
    return d >= BIG ? -1 : d;
}

// The K x K table, built from scratch by the oracle above.
inline std::vector<long> oracleMatrix(const gs::Grid &g,
                                      const std::vector<gs::Cell> &stops) {
    const int k = (int)stops.size();
    std::vector<long> m((size_t)k * k, gs::INF);
    for (int i = 0; i < k; ++i)
        for (int j = 0; j < k; ++j) {
            if (i == j) { m[(size_t)i * k + j] = 0; continue; }
            long d = cheapestCost(g, stops[i], stops[j]);
            m[(size_t)i * k + j] = d < 0 ? gs::INF : d;
        }
    return m;
}

// Greedy "always go to the nearest unvisited stop" tour cost.
inline long nearestNeighbourCost(const gs::Matrix &m) {
    std::vector<char> seen(m.k, 0);
    long total = 0;
    int at = 0;
    seen[0] = 1;
    for (int step = 1; step < m.k; ++step) {
        int best = -1;
        for (int j = 0; j < m.k; ++j)
            if (!seen[j] && m.at(at, j) < gs::INF &&
                (best < 0 || m.at(at, j) < m.at(at, best))) best = j;
        if (best < 0) return gs::INF;
        total += m.at(at, best);
        seen[best] = 1;
        at = best;
    }
    return total + m.at(at, 0);
}

// ---------------------------------------------------------------- recording

struct Event {
    char kind;              // 'P' push, 'E' expand, 'D' discard, 'R' relax
    int r = 0, c = 0;
    int pr = -1, pc = -1;   // parent, for E and R
};

struct Recording {
    std::vector<Event> events;
    std::vector<gs::Cell> expandedOrder;
};

// Run a search with the hooks recording, and hand back both the Result and
// the event stream.
inline Recording runRecorded(const gs::Grid &g, gs::Result &out) {
    gs::trace::begin(g, g.start(), g.goal(), gs::ALGORITHM, "test");
    out = gs::search(g, g.start(), g.goal());
    gs::trace::end(out.found ? out.path : std::vector<gs::Cell>{});

    Recording rec;
    std::istringstream in(gs::trace::text());
    std::string line;
    bool inEvents = false;
    while (std::getline(in, line)) {
        if (!inEvents) { if (line == "EVENTS") inEvents = true; continue; }
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::string tag; ls >> tag;
        if (tag == "RESULT" || tag == "W") break;
        Event e;
        e.kind = tag[0];
        if (e.kind == 'P') { long gg, ff; ls >> e.r >> e.c >> gg >> ff; }
        else if (e.kind == 'E') { ls >> e.r >> e.c >> e.pr >> e.pc; }
        else if (e.kind == 'D') { ls >> e.r >> e.c; }
        else if (e.kind == 'R') { long gg; ls >> e.r >> e.c >> gg >> e.pr >> e.pc; }
        else continue;
        if (e.kind == 'E') rec.expandedOrder.push_back(gs::Cell{e.r, e.c});
        rec.events.push_back(e);
    }
    return rec;
}

// ---------------------------------------------------------------- checks

// Every invariant that must hold of any correct search's recording.
// Returns "" when all is well, otherwise the first problem found.
inline std::string checkRecording(const gs::Grid &g, const gs::Result &res,
                                  const Recording &rec) {
    const int N = g.rows * g.cols;
    std::vector<int> pending(N, 0);
    std::vector<char> committed(N, 0);
    long live = 0, peak = 0, commits = 0;

    for (size_t i = 0; i < rec.events.size(); ++i) {
        const Event &e = rec.events[i];
        const int n = g.index(e.r, e.c);
        const std::string at = " (event " + std::to_string(i) + ")";
        if (!g.inBounds(e.r, e.c)) return "an event names a cell off the map" + at;

        switch (e.kind) {
            case 'P': case 'R':
                pending[n]++; live++;
                break;
            case 'D':
                if (pending[n] <= 0)
                    return "a cell was discarded that was not on the frontier" + at;
                pending[n]--; live--;
                break;
            case 'E':
                if (pending[n] <= 0)
                    return "a cell was committed to without ever joining the frontier" + at;
                if (committed[n])
                    return "the same cell was committed to twice" + at;
                if (e.pr >= 0 && !committed[g.index(e.pr, e.pc)])
                    return "a cell was committed to via a parent that had not been "
                           "committed to yet" + at;
                pending[n]--; live--;
                committed[n] = 1;
                commits++;
                break;
        }
        if (live < 0) return "the frontier count went negative" + at;
        peak = std::max(peak, live);
    }

    if (commits != res.expanded)
        return "you reported EXPANDED " + std::to_string(res.expanded) +
               " but committed to " + std::to_string(commits) + " cells";
    if (peak != res.maxFrontier)
        return "you reported MAXFRONTIER " + std::to_string(res.maxFrontier) +
               " but the recording peaks at " + std::to_string(peak);
    return "";
}

// The sequence of committed cells, as "r,c r,c r,c" -- for exact comparison.
inline std::string sequence(const Recording &rec) {
    std::string s;
    for (const gs::Cell &p : rec.expandedOrder) {
        if (!s.empty()) s += ' ';
        s += std::to_string(p.r) + "," + std::to_string(p.c);
    }
    return s;
}

// ---------------------------------------------------------------- the route

// The reference route for a shipped map, from expected/<stage>/<map>.path.
// found=false means the reference says no route exists.
struct RefPath { bool found=false; long cost=0; std::vector<gs::Cell> cells; };

inline RefPath referencePath(const std::string &stage, const std::string &map) {
    std::ifstream f(std::string(GS_EXPECTED) + "/" + stage + "/" + map + ".path");
    RefPath out;
    std::string first;
    if (!(f >> first)) return out;
    if (first == "NOPATH") return out;
    out.found = true;
    out.cost = std::stol(first);
    size_t n = 0;
    f >> n;
    out.cells.resize(n);
    for (size_t i = 0; i < n; ++i) f >> out.cells[i].r >> out.cells[i].c;
    return out;
}

// "" if the route is identical to the reference, otherwise where it diverges.
inline std::string comparePath(const std::vector<gs::Cell> &want,
                               const std::vector<gs::Cell> &got) {
    const size_t n = std::min(want.size(), got.size());
    for (size_t i = 0; i < n; ++i) {
        if (want[i] == got[i]) continue;
        return "step " + std::to_string(i) + " differs: the reference route goes to (" +
               std::to_string(want[i].r) + "," + std::to_string(want[i].c) +
               ") and yours goes to (" + std::to_string(got[i].r) + "," +
               std::to_string(got[i].c) + ") -- the first " + std::to_string(i) +
               " steps agree";
    }
    if (want.size() != got.size())
        return "the first " + std::to_string(n) + " steps agree, but the reference route "
               "is " + std::to_string(want.size()) + " cells and yours is " +
               std::to_string(got.size());
    return "";
}

// ---------------------------------------------------------------- full order

// The reference commit order for a shipped map, from expected/<stage>/<map>.seq.
inline std::vector<gs::Cell> referenceOrder(const std::string &stage,
                                            const std::string &map) {
    std::ifstream f(std::string(GS_EXPECTED) + "/" + stage + "/" + map + ".seq");
    std::vector<gs::Cell> out;
    int r, c;
    while (f >> r >> c) out.push_back(gs::Cell{r, c});
    return out;
}

// "" if the two orders are identical, otherwise where they first part ways.
inline std::string compareOrder(const std::vector<gs::Cell> &want,
                                const std::vector<gs::Cell> &got) {
    const size_t n = std::min(want.size(), got.size());
    for (size_t i = 0; i < n; ++i) {
        if (want[i] == got[i]) continue;
        return "commit " + std::to_string(i) + " differs: the reference commits to (" +
               std::to_string(want[i].r) + "," + std::to_string(want[i].c) +
               ") and you commit to (" + std::to_string(got[i].r) + "," +
               std::to_string(got[i].c) + ") -- the first " + std::to_string(i) +
               " commits agree";
    }
    if (want.size() != got.size())
        return "the first " + std::to_string(n) + " commits agree, but the reference "
               "commits to " + std::to_string(want.size()) + " cells and you commit to " +
               std::to_string(got.size());
    return "";
}

}  // namespace gstest

#endif
