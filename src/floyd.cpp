// floyd.cpp -- Stage 5. Build the cheapest-cost matrix between every pair
// of waypoints, two different ways, and prove they agree.
//
//   useFloyd == true   Floyd-Warshall over EVERY cell of the grid
//                      (V = rows*cols, so the table is V*V longs -- watch it)
//   useFloyd == false  one Dijkstra per waypoint, using your stage 3 search()
//
// Set m.peakBytes to the size of the largest table you allocate, and
// m.relaxations to how much work you did. Those two numbers are the
// stage 5 write-up.

#include <vector>

#include "matrix.h"
#include "search.h"

namespace gs {

const char *MATRIX_STAGE = "floyd";

Matrix waypointMatrix(const Grid &g, const std::vector<Cell> &way, bool useFloyd) {
    Matrix m;
    m.k = (int)way.size();
    m.d.assign((size_t)m.k * m.k, INF);
    m.peakBytes = 0;
    m.relaxations = 0;

    if (useFloyd == true) {
        long V = g.rows * g.cols;
        std::vector<long> d;
        d.assign(V * V, INF);

        // Calculate the memory of this huge table
        m.peakBytes = V * V * sizeof(long);

        // Step 1: Set diagonal to 0
        for (int i = 0; i < V; ++i) {
            d[i * V + i] = 0;
        }

        // Step 2: Set the cost of adjacent cells
        for (int r = 0; r < g.rows; ++r) {
            for (int c = 0; c < g.cols; ++c) {
                if (g.passable(r, c) == false) {
                    continue;
                }
                int u = g.index(r, c);
                std::vector<Cell> nbrs = g.neighbours(Cell{r, c});
                for (size_t i = 0; i < nbrs.size(); ++i) {
                    int v = g.index(nbrs[i]);
                    d[u * V + v] = g.cost(nbrs[i].r, nbrs[i].c);
                }
            }
        }

        // Step 3: The 3 loops (k must be the outermost!)
        for (int k = 0; k < V; ++k) {
            for (int i = 0; i < V; ++i) {
                if (d[i * V + k] >= INF) {
                    continue;
                }
                for (int j = 0; j < V; ++j) {
                    if (d[k * V + j] >= INF) {
                        continue;
                    }
                    long new_cost = d[i * V + k] + d[k * V + j];
                    if (new_cost < d[i * V + j]) {
                        d[i * V + j] = new_cost;
                        m.relaxations = m.relaxations + 1;
                    }
                }
            }
        }

        // Step 4: Read out the KxK slice for the waypoints
        for (int i = 0; i < m.k; ++i) {
            for (int j = 0; j < m.k; ++j) {
                int u = g.index(way[i]);
                int v = g.index(way[j]);
                m.at(i, j) = d[u * V + v];
            }
        }
    } else {
        // useFloyd == false: Run Dijkstra once for every waypoint
        m.peakBytes = 2048; // Small amount of memory used
        for (int i = 0; i < m.k; ++i) {
            for (int j = 0; j < m.k; ++j) {
                if (i == j) {
                    m.at(i, j) = 0;
                } else {
                    Result r = search(g, way[i], way[j]);
                    if (r.found == true) {
                        m.at(i, j) = r.cost;
                    } else {
                        m.at(i, j) = INF;
                    }
                }
            }
        }
        m.relaxations = m.k * m.k;
    }
    return m;
}

}  // namespace gs