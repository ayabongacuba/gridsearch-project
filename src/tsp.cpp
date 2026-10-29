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
    best.cost = INF;

    int k = (int)stops.size();
    best.order = {0, 0}; // Default for K=1

    if (k == 1) {
        best.cost = 0;
        best.order = {0, 0};
        return best;
    }

    // Build the matrix using repeated Dijkstra (useFloyd = false)
    Matrix m = waypointMatrix(g, stops, false);

    // Set up the permutation vector: [1, 2, ..., K-1]
    std::vector<int> perm;
    for (int i = 1; i < k; ++i) {
        perm.push_back(i);
    }

    long long candidate_count = 0;

    // Try every permutation
    do {
        candidate_count = candidate_count + 1;

        long current_cost = 0;
        int prev = 0;
        bool has_inf = false;

        for (size_t i = 0; i < perm.size(); ++i) {
            int next_stop = perm[i];
            long leg_cost = m.at(prev, next_stop);

            if (leg_cost >= INF) {
                has_inf = true;
                break;
            }
            current_cost = current_cost + leg_cost;
            prev = next_stop;
        }

        if (has_inf == true) {
            continue;
        }

        // Add the leg back to the depot (waypoint 0)
        long return_cost = m.at(prev, 0);
        if (return_cost >= INF) {
            continue;
        }
        current_cost = current_cost + return_cost;

        // Keep strictly cheaper
        if (current_cost < best.cost) {
            best.cost = current_cost;

            best.order.clear();
            best.order.push_back(0);
            for (size_t i = 0; i < perm.size(); ++i) {
                best.order.push_back(perm[i]);
            }
            best.order.push_back(0);
        }

    } while (std::next_permutation(perm.begin(), perm.end()) == true);

    best.candidates = candidate_count;
    return best;
}

}  // namespace gs