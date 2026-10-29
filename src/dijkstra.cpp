// dijkstra.cpp -- Stage 3. Find the cheapest path when terrain costs differ.
//
// Fill in search(). Everything else is provided.
//
//   Result fields you must set: found, path, cost, expanded, maxFrontier
//   path runs start first, goal last.
//   cost is the sum of g.cost(r, c) over every cell entered AFTER the start.
//
//   Frontier: a std::priority_queue ordered by cost-so-far. The standard
//   one has no decrease-key, so push a fresh entry and skip stale copies
//   when they come off the top -- call trace::discard when you skip one.
//
//   Call the trace hooks as you go -- trace::push when a cell enters the
//   frontier, trace::expand when one is committed, trace::discard when a
//   popped cell is thrown away. Then run with --viz and watch it.

#include <algorithm>
#include <vector>
#include <queue>

#include "search.h"
#include "trace.h"

namespace gs {

const char *ALGORITHM = "dijkstra";

struct State {
    long cost;
    int r;
    int c;
};

struct CompareState {
    bool operator()(const State &a, const State &b) const {
        if (a.cost != b.cost) {
            return a.cost > b.cost; // Min-heap: smaller cost is higher priority
        }
        if (a.r != b.r) {
            return a.r > b.r; // Tie-breaker: smaller row
        }
        return a.c > b.c; // Tie-breaker: smaller column
    }
};

Result search(const Grid &g, Cell start, Cell goal) {
    Result out;
    out.found = false;
    out.cost = 0;
    out.expanded = 0;
    out.maxFrontier = 0;

    std::priority_queue<State, std::vector<State>, CompareState> pq;

    std::vector<long> dist;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        dist.push_back(INF);
    }

    std::vector<bool> settled;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        settled.push_back(false);
    }

    std::vector<Cell> parent_cells;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        Cell empty_cell;
        empty_cell.r = -1;
        empty_cell.c = -1;
        parent_cells.push_back(empty_cell);
    }

    State start_state;
    start_state.cost = 0;
    start_state.r = start.r;
    start_state.c = start.c;

    dist[g.index(start)] = 0;
    pq.push(start_state);
    trace::push(start.r, start.c, 0, 0);

    if (pq.size() > (size_t)out.maxFrontier) {
        out.maxFrontier = pq.size();
    }

    while (pq.empty() == false) {
        State curr_state = pq.top();
        pq.pop();

        int curr_idx = g.index(curr_state.r, curr_state.c);

        if (settled[curr_idx] == true) {
            trace::discard(curr_state.r, curr_state.c);
            continue;
        }

        settled[curr_idx] = true;
        out.expanded++;

        Cell curr;
        curr.r = curr_state.r;
        curr.c = curr_state.c;

        int pr = -1;
        int pc = -1;
        Cell p = parent_cells[curr_idx];
        if (p.r != -1 && p.c != -1) {
            pr = p.r;
            pc = p.c;
        }

        trace::expand(curr.r, curr.c, pr, pc);

        if (curr.r == goal.r && curr.c == goal.c) {
            out.found = true;
            out.cost = curr_state.cost;
            break;
        }

        std::vector<Cell> nbrs = g.neighbours(curr);
        for (size_t i = 0; i < nbrs.size(); ++i) {
            Cell nbr = nbrs[i];
            int nbr_idx = g.index(nbr);

            if (settled[nbr_idx] == true) {
                continue;
            }

            long new_cost = curr_state.cost + g.cost(nbr.r, nbr.c);

            if (new_cost < dist[nbr_idx]) {
                dist[nbr_idx] = new_cost;
                parent_cells[nbr_idx] = curr;

                State nbr_state;
                nbr_state.cost = new_cost;
                nbr_state.r = nbr.r;
                nbr_state.c = nbr.c;

                pq.push(nbr_state);
                trace::push(nbr.r, nbr.c, new_cost, new_cost);
            }
        }

        if (pq.size() > (size_t)out.maxFrontier) {
            out.maxFrontier = pq.size();
        }
    }

    if (out.found == true) {
        std::vector<Cell> reverse_path;
        int curr_idx = g.index(goal);

        while (curr_idx != -1) {
            Cell c = g.at(curr_idx);
            reverse_path.push_back(c);

            if (c.r == start.r && c.c == start.c) {
                break;
            }

            Cell p = parent_cells[curr_idx];
            if (p.r == -1 && p.c == -1) {
                break; // Should not happen if found is true, but safe
            }
            curr_idx = g.index(p);
        }

        for (int i = (int)reverse_path.size() - 1; i >= 0; --i) {
            out.path.push_back(reverse_path[i]);
        }
    }

    return out;
}

}  // namespace gs