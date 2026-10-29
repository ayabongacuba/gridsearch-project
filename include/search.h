// search.h -- the one function you write for each stage.
// Provided. You should not need to change this file.

#ifndef GS_SEARCH_H
#define GS_SEARCH_H

#include <vector>

#include "grid.h"

namespace gs {

struct Result {
    bool found = false;
    std::vector<Cell> path;    // start first, goal last
    long cost = 0;             // sum of the cost of every cell entered after the start
    long expanded = 0;         // cells taken off the frontier and committed
    long maxFrontier = 0;      // largest the frontier ever got
};

// Implement this in your stage file (src/dfs.cpp, src/bfs.cpp, ...).
Result search(const Grid &g, Cell start, Cell goal);

// The name shown in the visualiser. Define it in your stage file too.
extern const char *ALGORITHM;

}  // namespace gs

#endif
