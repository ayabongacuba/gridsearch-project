// tsp.cpp -- Stage 6 baseline. Given the waypoint matrix, find the cheapest
// closed tour that starts at waypoint 0, visits every other waypoint exactly
// once, and comes back.
//
// The baseline is exhaustive: fix waypoint 0, permute the rest, keep the
// best. std::next_permutation does the permuting. Set best.candidates to how
// many orderings you actually looked at -- that is the number you plot.

#include <algorithm>
#include <numeric>
#include <vector>

#include "matrix.h"

namespace gs {

const char *TOUR_STAGE = "tsp-brute";

Tour bestTour(const Grid &g, const std::vector<Cell> &stops) {
    Tour best;

    // TODO: your search here.
    //
    // You are handed the map and the stops, and nothing else. What you need
    // is the K x K table of costs between every pair of stops -- and you
    // wrote two ways of building one in Stage 5.
    //
    // best.order should read 0, ..., 0 -- the depot at both ends.
    // best.cost is the sum of the legs along it.
    // Treat any entry >= INF as unreachable and skip that ordering.

    (void)g; (void)stops;
    return best;
}

}  // namespace gs
