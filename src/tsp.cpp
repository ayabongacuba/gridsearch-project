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
#include <chrono>

#include "matrix.h"

namespace gs {

const char *TOUR_STAGE = "tsp-2opt";

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

    auto start_time = std::chrono::steady_clock::now();

    // Build the matrix using repeated Dijkstra (useFloyd = false)
    Matrix m = waypointMatrix(g, stops, false);

    if (k <= 11) {
        // --- STAGE 6 BRUTE FORCE (THE GATE) ---
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

    } else {
        // --- PHASE 3 HEURISTIC ---

        // Check for unreachable stops first
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                if (i != j && m.at(i, j) >= INF) {
                    best.cost = INF;
                    return best;
                }
            }
        }

        // Create the symmetric matrix
        std::vector<long> sym_table(k * k);
        for (int i = 0; i < k; ++i) {
            long cost_i = g.cost(stops[i].r, stops[i].c);
            for (int j = 0; j < k; ++j) {
                sym_table[i * k + j] = m.at(i, j) + cost_i;
            }
        }

        // Check if the matrix is actually symmetric (safety check)
        bool symmetric = true;
        for (int i = 0; i < k && symmetric; ++i) {
            for (int j = i + 1; j < k; ++j) {
                if (sym_table[i * k + j] != sym_table[j * k + i]) {
                    symmetric = false;
                    break;
                }
            }
        }

        long overall_best_cost = INF;
        std::vector<int> overall_best_order;

        // Multi-Start 2-Opt
        for (int start_node = 0; start_node < k; ++start_node) {

            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - start_time;
            if (elapsed.count() > 1.8) {
                break;
            }

            // Build Nearest Neighbour tour from start_node
            std::vector<int> current_order;
            std::vector<bool> visited(k, false);

            int current_node = start_node;
            current_order.push_back(current_node);
            visited[current_node] = true;

            for (int step = 1; step < k; ++step) {
                int best_next = -1;
                long best_dist = INF;

                for (int j = 0; j < k; ++j) {
                    if (visited[j] == false) {
                        long dist = sym_table[current_node * k + j];
                        if (dist < best_dist) {
                            best_dist = dist;
                            best_next = j;
                        }
                    }
                }

                current_order.push_back(best_next);
                visited[best_next] = true;
                current_node = best_next;
            }
            current_order.push_back(start_node); // Close the cycle

            // Rotate the tour so it starts at 0
            std::vector<int> best_order;
            int zero_index = -1;
            for (size_t i = 0; i < current_order.size() - 1; ++i) {
                if (current_order[i] == 0) {
                    zero_index = i;
                    break;
                }
            }
            if (zero_index != -1) {
                for (size_t i = zero_index; i < current_order.size() - 1; ++i) {
                    best_order.push_back(current_order[i]);
                }
                for (int i = 0; i < zero_index; ++i) {
                    best_order.push_back(current_order[i]);
                }
                best_order.push_back(0);
            }

            // 2-Opt Local Search
            if (symmetric == true) {
                bool improved = true;
                bool time_up = false;

                while (improved == true && time_up == false) {
                    improved = false;
                    int check_counter = 0;

                    for (int i = 1; i < k - 1 && time_up == false; ++i) {
                        for (int j = i + 1; j < k; ++j) {

                            check_counter = check_counter + 1;
                            // Check the clock inside the loop every 1000 moves
                            if (check_counter % 1000 == 0) {
                                auto current_time = std::chrono::steady_clock::now();
                                std::chrono::duration<double> elapsed2 = current_time - start_time;
                                if (elapsed2.count() > 1.8) {
                                    time_up = true;
                                    break;
                                }
                            }

                            int a = best_order[i - 1];
                            int b = best_order[i];
                            int c = best_order[j];
                            int d = best_order[j + 1];

                            long delta = (sym_table[a * k + c] + sym_table[b * k + d])
                                       - (sym_table[a * k + b] + sym_table[c * k + d]);

                            if (delta < 0) {
                                std::reverse(best_order.begin() + i, best_order.begin() + j + 1);
                                improved = true;
                            }
                        }
                    }
                }
            }

            // Calculate the true asymmetric cost to compare
            long current_true_cost = 0;
            for (size_t i = 0; i + 1 < best_order.size(); ++i) {
                current_true_cost += m.at(best_order[i], best_order[i + 1]);
            }

            if (current_true_cost < overall_best_cost) {
                overall_best_cost = current_true_cost;
                overall_best_order = best_order;
            }
        }

        // Fallback if no tour was found (e.g., timed out on the first one)
        if (overall_best_order.empty()) {
            std::vector<int> current_order;
            std::vector<bool> visited(k, false);
            current_order.push_back(0);
            visited[0] = true;
            int current_node = 0;
            for (int step = 1; step < k; ++step) {
                int best_next = -1;
                long best_dist = INF;
                for (int j = 0; j < k; ++j) {
                    if (visited[j] == false && sym_table[current_node * k + j] < best_dist) {
                        best_dist = sym_table[current_node * k + j];
                        best_next = j;
                    }
                }
                current_order.push_back(best_next);
                visited[best_next] = true;
                current_node = best_next;
            }
            current_order.push_back(0);
            overall_best_order = current_order;
            overall_best_cost = 0;
            for (size_t i = 0; i + 1 < overall_best_order.size(); ++i) {
                overall_best_cost += m.at(overall_best_order[i], overall_best_order[i + 1]);
            }
        }

        best.cost = overall_best_cost;
        best.order = overall_best_order;
        best.candidates = 0;
        return best;
    }
}

}  // namespace gs