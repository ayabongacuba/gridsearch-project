// dfs.cpp -- Stage 1. Find A path between the two cells. Not the shortest one.
//
// Fill in search(). Everything else is provided.
//
//   Result fields you must set: found, path, cost, expanded, maxFrontier
//   path runs start first, goal last.
//   cost is the sum of g.cost(r, c) over every cell entered AFTER the start.
//
//   Frontier: a std::vector used as a stack. Not recursion -- one of the
//   test grids is deep enough to overflow the call stack, and MAXFRONTIER
//   is meaningless if the frontier is the call stack.
//
//   Push neighbours in the order N, E, S, W (use DR[d], DC[d]).
//
//   Call the trace hooks as you go -- trace::push when a cell enters the
//   frontier, trace::expand when one is committed, trace::discard when a
//   popped cell is thrown away. Then run with --viz and watch it.

#include <algorithm>
#include <vector>

#include "search.h"
#include "trace.h"

namespace gs {

const char *ALGORITHM = "dfs";

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
