// trace.h -- search event recorder.
//
// Call these four hooks from inside your search loop. When the program is
// run without --viz or --trace every hook is a no-op, so leave the calls in
// even for the timed runs.
//
// Deliberately absent: counters. Result::expanded and Result::maxFrontier
// are yours to compute in your own code. The marker cross-checks what you
// report against this event stream, so the two have to agree.
//
// Provided. DO NOT MODIFY THIS FILE.

#ifndef GS_TRACE_H
#define GS_TRACE_H

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "grid.h"

namespace gs {
namespace trace {

inline std::ostringstream &buf() { static std::ostringstream s; return s; }
inline bool &on() { static bool b = false; return b; }
inline long &count() { static long n = 0; return n; }

// --- live streaming -------------------------------------------------------
// When the driver is run with --live, each event is also handed to this sink
// as it happens, and the search is paced so a human can watch it. Pacing is
// done here, in provided code, so your search loop stays untouched.
using Sink = void (*)(const std::string &);
inline Sink &sink() { static Sink s = nullptr; return s; }
inline double &pace() { static double p = 0; return p; }      // events/second
inline long long &sleptMs() { static long long m = 0; return m; }

inline void flushEvent() {
    if (!sink()) return;
    std::string all = buf().str();
    buf().str("");
    sink()(all);
    if (pace() > 0) {
        static std::chrono::steady_clock::time_point t0;
        static long budget = 0;
        if (budget == 0) t0 = std::chrono::steady_clock::now();
        if (++budget >= 32) {
            auto target = std::chrono::microseconds(
                (long long)(budget / pace() * 1e6));
            auto spent = std::chrono::steady_clock::now() - t0;
            if (spent < target) {
                auto nap = target - std::chrono::duration_cast<
                    std::chrono::microseconds>(spent);
                sleptMs() += std::chrono::duration_cast<
                    std::chrono::milliseconds>(nap).count();
                std::this_thread::sleep_for(nap);
            }
            budget = 0;
        }
    }
}

// Guard against a runaway search filling memory with events.
inline constexpr long EVENT_LIMIT = 4000000;

inline void begin(const Grid &g, Cell s, Cell t, const char *algorithm,
                  const std::string &instance) {
    on() = true;
    count() = 0;
    buf().str("");
    buf() << "ALGORITHM " << algorithm << '\n'
          << "INSTANCE " << instance << '\n'
          << "GRID " << g.rows << ' ' << g.cols << '\n'
          << "MAP\n";
    for (const std::string &row : g.map) buf() << row << '\n';
    buf() << "START " << s.r << ' ' << s.c << '\n'
          << "GOAL " << t.r << ' ' << t.c << '\n'
          << "EVENTS\n";
    flushEvent();
}

inline bool room() {
    if (!on()) return false;
    if (++count() > EVENT_LIMIT) { on() = false; return false; }
    return true;
}

// A cell has been placed on the frontier. g is cost-so-far, f is the
// priority you queued it under. For DFS and BFS pass the depth for both.
inline void push(int r, int c, long gcost, long f) {
    if (!room()) return;
    buf() << "P " << r << ' ' << c << ' ' << gcost << ' ' << f << '\n';
    flushEvent();
}

// A cell has been taken off the frontier and committed. Pass the cell you
// reached it from; use (-1, -1) for the start.
inline void expand(int r, int c, int pr, int pc) {
    if (!room()) return;
    buf() << "E " << r << ' ' << c << ' ' << pr << ' ' << pc << '\n';
    flushEvent();
}

// A cell came off the frontier but was thrown away without expanding it:
// already visited, or a stale priority-queue entry.
inline void discard(int r, int c) {
    if (!room()) return;
    buf() << "D " << r << ' ' << c << '\n';
    flushEvent();
}

// A cheaper route to a cell already on the frontier was found.
// Stages 3 and 4 only.
inline void relax(int r, int c, long gcost, int pr, int pc) {
    if (!room()) return;
    buf() << "R " << r << ' ' << c << ' ' << gcost << ' ' << pr << ' ' << pc << '\n';
    flushEvent();
}

inline void end(const std::vector<Cell> &path) {
    if (!on()) return;
    buf() << "RESULT " << (path.empty() ? "NOPATH" : "PATH") << '\n';
    for (const Cell &p : path) buf() << "W " << p.r << ' ' << p.c << '\n';
    flushEvent();
}

inline std::string text() { return buf().str(); }
inline bool truncated() { return count() > EVENT_LIMIT; }

}  // namespace trace
}  // namespace gs

#endif
