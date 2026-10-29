// util.cpp -- Stage 0.
//
// Implement everything declared in include/grid.h. Nothing else in the
// project works until this does.

#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "grid.h"

namespace gs {

// "No route exists". Half of LONG_MAX so that INF + INF still fits.
const long INF = std::numeric_limits<long>::max() / 4;

bool Grid::inBounds(int r, int c) const {
    if (r >= 0 && r < rows) {
        if (c >= 0 && c < cols) {
            return true;
        }
    }
    return false;
}

bool Grid::passable(int r, int c) const {
    if (inBounds(r, c) == false) {
        return false;
    }
    if (map[r][c] == '#') {
        return false;
    }
    return true;
}

long Grid::cost(int r, int c) const {
    char ch = map[r][c];
    if (ch == '.') {
        return 1;
    }
    long val = ch - '0';
    return val;
}

int Grid::index(int r, int c) const {
    return r * cols + c;
}

int Grid::index(Cell x) const {
    return x.r * cols + x.c;
}

Cell Grid::at(int i) const {
    int r = i / cols;
    int c = i % cols;
    Cell result;
    result.r = r;
    result.c = c;
    return result;
}

std::vector<Cell> Grid::neighbours(Cell x) const {
    std::vector<Cell> result;

    for (int d = 0; d < 4; ++d) {
        int nr = x.r + DR[d];
        int nc = x.c + DC[d];

        if (passable(nr, nc) == true) {
            Cell neighbor;
            neighbor.r = nr;
            neighbor.c = nc;
            result.push_back(neighbor);
        }
    }

    return result;
}

Cell Grid::start() const {
    return waypoints[0];
}

Cell Grid::goal() const {
    return waypoints[1];
}

Grid readGrid(std::istream &in) {
    Grid g;

    in >> g.rows >> g.cols;

    std::string dummy;
    std::getline(in, dummy);

    for (int i = 0; i < g.rows; ++i) {
        std::string row;
        std::getline(in, row);

        if (row.length() != (size_t)g.cols) {
            std::cerr << "Malformed row" << std::endl;
            std::exit(2);
        }
        g.map.push_back(row);
    }

    int k;
    in >> k;

    if (k < 1) {
        std::cerr << "Must have at least 1 stop" << std::endl;
        std::exit(2);
    }

    for (int i = 0; i < k; ++i) {
        int r;
        int c;
        in >> r >> c;

        Cell stop;
        stop.r = r;
        stop.c = c;
        g.waypoints.push_back(stop);
    }

    return g;
}

std::vector<long> makeCostTable(int k) {
    std::vector<long> table;

    for (int i = 0; i < k * k; ++i) {
        table.push_back(INF);
    }

    for (int i = 0; i < k; ++i) {
        int idx = i * k + i;
        table[idx] = 0;
    }

    return table;
}

std::vector<std::string> renderPath(const Grid &g,
                                    const std::vector<Cell> &path) {
    std::vector<std::string> result = g.map;

    for (size_t i = 0; i < path.size(); ++i) {
        int r = path[i].r;
        int c = path[i].c;

        if (i == 0) {
            result[r][c] = 'S';
        } else if (i == path.size() - 1) {
            result[r][c] = 'G';
        } else {
            int pr = path[i - 1].r;
            int pc = path[i - 1].c;

            if (r < pr) {
                result[r][c] = '^';
            } else if (r > pr) {
                result[r][c] = 'v';
            } else if (c < pc) {
                result[r][c] = '<';
            } else if (c > pc) {
                result[r][c] = '>';
            }
        }
    }

    return result;
}

}  // namespace gs