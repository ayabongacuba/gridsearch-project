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

    // TODO: fill m.at(i, j) with the cheapest cost from waypoint i to j.
    //
    // For the Dijkstra route, search(g, way[i], way[j]) is your stage 3 code
    // and already returns the cost you need.
    //
    // For the Floyd-Warshall route: index cells with g.index(r, c), seed the
    // table with the 4-neighbour edges, then run the three nested loops with
    // the intermediate vertex OUTERMOST. Getting that loop order wrong is the
    // classic mistake and it produces answers that are quietly too large.

    (void)g; (void)useFloyd;
    return m;
}

}  // namespace gs
