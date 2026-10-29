// bfs.cpp -- Stage 2. Find a path with the fewest steps.
//
// Fill in search(). Everything else is provided.
//
//   Result fields you must set: found, path, cost, expanded, maxFrontier
//   path runs start first, goal last.
//   cost is the sum of g.cost(r, c) over every cell entered AFTER the start.
//
//   Frontier: a queue. Mark cells seen when you PUSH them, not when you
//   pop them, or the same cell lands in the queue several times.
//
//   Call the trace hooks as you go -- trace::push when a cell enters the
//   frontier, trace::expand when one is committed, trace::discard when a
//   popped cell is thrown away. Then run with --viz and watch it.

#include <algorithm>
#include <vector>

#include "search.h"
#include "trace.h"

namespace gs {

const char *ALGORITHM = "bfs";

Result search(const Grid &g, Cell start, Cell goal) {
    Result out;

    // TODO: your search here.
    //
    // Reconstructing the path: keep a parent array of ints, set parent[n]
    // when you EXPAND n, then walk back from g.index(goal) to -1 and
    // std::reverse the result.

    (void)g; (void)start; (void)goal;
    return out;
}

}  // namespace gs
