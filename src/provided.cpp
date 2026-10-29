// provided.cpp -- helpers you are given. Do not modify.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "grid.h"

namespace gs {

Grid readGridFile(const std::string &path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "map: cannot open " << path << "\n";
        std::exit(2);
    }
    return readGrid(f);
}

std::string validatePath(const Grid &g, const std::vector<Cell> &path,
                         long declaredCost) {
    if (path.empty()) return "the route is empty";
    if (path.front() != g.start()) return "the route does not begin at the start";
    if (path.back() != g.goal()) return "the route does not end at the destination";
    std::vector<char> seen((size_t)g.rows * g.cols, 0);
    long total = 0;
    for (size_t i = 0; i < path.size(); ++i) {
        const Cell &p = path[i];
        if (!g.inBounds(p.r, p.c))
            return "step " + std::to_string(i) + " is off the map";
        if (!g.passable(p.r, p.c))
            return "step " + std::to_string(i) + " is inside a building";
        if (seen[g.index(p)])
            return "step " + std::to_string(i) + " revisits a cell";
        seen[g.index(p)] = 1;
        if (i) {
            int dr = std::abs(path[i].r - path[i - 1].r);
            int dc = std::abs(path[i].c - path[i - 1].c);
            if (dr + dc != 1)
                return "step " + std::to_string(i) +
                       " is not adjacent to the one before it";
            total += g.cost(p.r, p.c);
        }
    }
    if (total != declaredCost)
        return "you reported a cost of " + std::to_string(declaredCost) +
               " but the route you printed costs " + std::to_string(total);
    return "";
}

}  // namespace gs
