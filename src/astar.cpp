// astar.cpp -- Stage 4. Find the cheapest path, guided by a heuristic. Same cost as Dijkstra, fewer expansions.
//
// Fill in search(). Everything else is provided.
//
//   Result fields you must set: found, path, cost, expanded, maxFrontier
//   path runs start first, goal last.
//   cost is the sum of g.cost(r, c) over every cell entered AFTER the start.
//
//   Start from your Dijkstra and change the priority to g + h.
//   Write h as a separate function so you can swap it out later.
//   Manhattan distance is admissible here. Convince yourself why before
//   you use it.
//
//   Call the trace hooks as you go -- trace::push when a cell enters the
//   frontier, trace::expand when one is committed, trace::discard when a
//   popped cell is thrown away. Then run with --viz and watch it.

#include <algorithm>
#include <vector>

#include "search.h"
#include "trace.h"

namespace gs {

const char *ALGORITHM = "astar";

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
