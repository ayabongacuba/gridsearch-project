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
    out.found = false;
    out.cost = 0;
    out.expanded = 0;
    out.maxFrontier = 0;

    struct StackItem {
        Cell cell;
        Cell parent;
        bool has_parent;
    };

    std::vector<StackItem> stack;

    std::vector<bool> visited;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        visited.push_back(false);
    }

    std::vector<Cell> parent_cells;
    for (int i = 0; i < g.rows * g.cols; ++i) {
        Cell empty_cell;
        empty_cell.r = -1;
        empty_cell.c = -1;
        parent_cells.push_back(empty_cell);
    }

    StackItem start_item;
    start_item.cell = start;
    start_item.has_parent = false;
    stack.push_back(start_item);
    trace::push(start.r, start.c, 0, 0);

    if (stack.size() > (size_t)out.maxFrontier) {
        out.maxFrontier = stack.size();
    }

    while (stack.empty() == false) {
        StackItem curr_item = stack.back();
        stack.pop_back();

        Cell curr = curr_item.cell;
        int curr_idx = g.index(curr);

        if (visited[curr_idx] == true) {
            trace::discard(curr.r, curr.c);
            continue;
        }

        visited[curr_idx] = true;
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
            if (visited[nbr_idx] == false) {
                StackItem nbr_item;
                nbr_item.cell = nbr;
                nbr_item.parent = curr;
                nbr_item.has_parent = true;
                stack.push_back(nbr_item);
                trace::push(nbr.r, nbr.c, 0, 0);
            }
        }

        if (stack.size() > (size_t)out.maxFrontier) {
            out.maxFrontier = stack.size();
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