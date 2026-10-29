// bfs.cpp -- Stage 2. Find a path with the fewest steps.
//
// Fill in search(). Everything else is provided.
//
//   Result fields you must set: found, path, cost, expanded, maxFrontier
//   path runs start first, goal last.
//   cost is the sum of g.cost(r, c) over every cell entered AFTER the start.
//
//   Frontier: a queue. Mark cells seen when you PUSH them, not when you
//   pop them, or the same cell lands in the queue several times.
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

const char *ALGORITHM = "bfs";

Result search(const Grid &g, Cell start, Cell goal) {
    Result out;
    out.found = false;
    out.cost = 0;
    out.expanded = 0;
    out.maxFrontier = 0;

    struct QueueItem {
        Cell cell;
        Cell parent;
        bool has_parent;
    };

    std::queue<QueueItem> q;

    std::vector<bool> seen;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        seen.push_back(false);
    }

    std::vector<Cell> parent_cells;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        Cell empty_cell;
        empty_cell.r = -1;
        empty_cell.c = -1;
        parent_cells.push_back(empty_cell);
    }

    QueueItem start_item;
    start_item.cell = start;
    start_item.has_parent = false;
    q.push(start_item);
    seen[g.index(start)] = true;
    trace::push(start.r, start.c, 0, 0);

    if (q.size() > (size_t)out.maxFrontier) {
        out.maxFrontier = q.size();
    }

    while (q.empty() == false) {
        QueueItem curr_item = q.front();
        q.pop();

        Cell curr = curr_item.cell;
        int curr_idx = g.index(curr);

        out.expanded++;

        int pr = -1;
        int pc = -1;
        if (curr_item.has_parent == true) {
            pr = curr_item.parent.r;
            pc = curr_item.parent.c;
            parent_cells[curr_idx] = curr_item.parent;
        }

        trace::expand(curr.r, curr.c, pr, pc);

        if (curr.r == goal.r && curr.c == goal.c) {
            out.found = true;
            break;
        }

        std::vector<Cell> nbrs = g.neighbours(curr);
        for (size_t i = 0; i < nbrs.size(); ++i) {
            Cell nbr = nbrs[i];
            int nbr_idx = g.index(nbr);

            if (seen[nbr_idx] == false) {
                seen[nbr_idx] = true;

                QueueItem nbr_item;
                nbr_item.cell = nbr;
                nbr_item.parent = curr;
                nbr_item.has_parent = true;
                q.push(nbr_item);
                trace::push(nbr.r, nbr.c, 0, 0);
            }
        }

        if (q.size() > (size_t)out.maxFrontier) {
            out.maxFrontier = q.size();
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
            curr_idx = g.index(p);
        }

        for (int i = (int)reverse_path.size() - 1; i >= 0; --i) {
            out.path.push_back(reverse_path[i]);
        }

        for (size_t i = 1; i < out.path.size(); ++i) {
            out.cost += g.cost(out.path[i].r, out.path[i].c);
        }
    }

    return out;
}

}  // namespace gs