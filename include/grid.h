// grid.h -- the map, and the operations every stage needs from it.
//
// This header DECLARES the interface. You implement it in src/util.cpp
// (Stage 0). Do not modify this file.

#ifndef GS_GRID_H
#define GS_GRID_H

#include <iostream>
#include <string>
#include <vector>

namespace gs {

// A position on the map. Row 0 is the top, column 0 is the left.
struct Cell {
    int r = 0, c = 0;
    bool operator==(const Cell &o) const { return r == o.r && c == o.c; }
    bool operator!=(const Cell &o) const { return !(*this == o); }
};

// Neighbour order for the entire project: North, East, South, West.
// Every stage must consider adjacent cells in this order.
inline constexpr int DR[4] = {-1, 0, 1, 0};
inline constexpr int DC[4] = {0, 1, 0, -1};

// The value used for "no route exists". Large enough that INF + INF does not
// overflow a long, so you can add two of them without the result wrapping.
extern const long INF;

class Grid {
public:
    int rows = 0, cols = 0;
    std::vector<std::string> map;      // map[r][c] is '#', '.', or '1'-'9'
    std::vector<Cell> waypoints;       // waypoints[0] is the start / depot

    // --- Stage 0: you implement all of these in src/util.cpp ---

    // Is (r, c) inside the map at all?
    bool inBounds(int r, int c) const;

    // Is (r, c) on the map AND not a building?
    bool passable(int r, int c) const;

    // What it costs to ENTER this cell. '.' costs 1; '1'-'9' cost their
    // value. Never called on a building.
    long cost(int r, int c) const;

    // A cell as a single integer, and back again. Use these to index flat
    // vectors of size rows*cols instead of nesting vectors.
    int index(int r, int c) const;
    int index(Cell x) const;
    Cell at(int i) const;

    // The walkable neighbours of a cell, in N, E, S, W order, skipping
    // buildings and anything off the map.
    std::vector<Cell> neighbours(Cell x) const;

    Cell start() const;                // waypoints[0]
    Cell goal() const;                 // waypoints[1]
};

// --- Stage 0: you implement these too ---

// Read a map in the format described in the brief.
Grid readGrid(std::istream &in);

// Allocate a k-by-k table of costs, every entry INF except a zero diagonal.
// Stored row-major: entry (i, j) is at index i*k + j.
std::vector<long> makeCostTable(int k);

// Draw a route onto a copy of the map: '^' '>' 'v' '<' in the direction of
// travel, 'S' at the start and 'G' at the destination.
std::vector<std::string> renderPath(const Grid &g, const std::vector<Cell> &path);

// --- provided for you, in src/provided.cpp ---

// Opens a file and calls your readGrid on it.
Grid readGridFile(const std::string &path);

// Returns "" if the route is legal, otherwise a description of the first
// fault. This is the same check the marker runs -- use it while you work.
std::string validatePath(const Grid &g, const std::vector<Cell> &path,
                         long declaredCost);

}  // namespace gs

#endif
