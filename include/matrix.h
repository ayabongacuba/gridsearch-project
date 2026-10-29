// matrix.h -- the types stages 5 and 6 hand back.
// Provided. You should not need to change this file.

#ifndef GS_MATRIX_H
#define GS_MATRIX_H

#include <vector>

#include "grid.h"

namespace gs {

// Cheapest cost between every pair of waypoints. Row-major, k by k.
struct Matrix {
    int k = 0;
    std::vector<long> d;
    long peakBytes = 0;        // the biggest table you allocated
    long long relaxations = 0; // how much work it took, for the write-up
    long at(int i, int j) const { return d[(size_t)i * k + j]; }
    long &at(int i, int j) { return d[(size_t)i * k + j]; }
};

// Stage 5. useFloyd picks which way you build it.
Matrix waypointMatrix(const Grid &g, const std::vector<Cell> &way, bool useFloyd);
extern const char *MATRIX_STAGE;

// Stage 6. A closed tour that starts and ends at waypoint 0.
struct Tour {
    std::vector<int> order;      // 0, ..., 0
    long cost = 0;
    long long candidates = 0;    // how many orderings you actually tried
};

// Note what this does NOT receive. Working out what you need, and where to
// get it, is the point of Stage 6.
Tour bestTour(const Grid &g, const std::vector<Cell> &stops);
extern const char *TOUR_STAGE;

}  // namespace gs

#endif
