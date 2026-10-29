#!/usr/bin/env python3
"""
maze_gen.py -- instance generator for the grid-search project.

Emits the canonical input format:

    R C
    <R lines of grid: '#' wall, '.' open (cost 1)>
    K
    r c        (K waypoint lines; waypoint 0 = start, 1 = goal for stages 1-4)

Styles
    perfect   recursive-backtracker maze. Exactly one path between any two
              cells. DFS and BFS return the SAME path -- useful as a control.
    braid     perfect maze with dead ends knocked through. Multiple routes,
              so DFS/BFS length ratio becomes interesting.
    rooms     chambers joined by corridors. Large open areas: frontier grows
              fast, DFS wanders.
    caves     cellular-automata cavern. Organic, wide, very open.
    comb      adversarial. DFS explores nearly every cell before finding a
              goal that sits a few steps away.

Usage
    python3 maze_gen.py --style perfect --size 41 --seed 7 --out m.grid
    python3 maze_gen.py --suite --outdir mazes/
"""

import argparse
import json
import os
import random
from collections import deque

WALL, OPEN = "#", "."


def blank(r, c, fill=WALL):
    return [[fill] * c for _ in range(r)]


# ---------------------------------------------------------------- generators

def gen_perfect(r, c, rng):
    """Recursive backtracker on odd coordinates."""
    g = blank(r, c)
    start = (1, 1)
    g[1][1] = OPEN
    stack = [start]
    while stack:
        cr, cc = stack[-1]
        nbrs = []
        for dr, dc in ((-2, 0), (2, 0), (0, -2), (0, 2)):
            nr, nc = cr + dr, cc + dc
            if 1 <= nr < r - 1 and 1 <= nc < c - 1 and g[nr][nc] == WALL:
                nbrs.append((nr, nc, dr, dc))
        if not nbrs:
            stack.pop()
            continue
        nr, nc, dr, dc = rng.choice(nbrs)
        g[cr + dr // 2][cc + dc // 2] = OPEN
        g[nr][nc] = OPEN
        stack.append((nr, nc))
    return g


def gen_braid(r, c, rng, braid=0.35):
    """Perfect maze, then remove a fraction of dead ends to create loops."""
    g = gen_perfect(r, c, rng)
    deads = []
    for i in range(1, r - 1):
        for j in range(1, c - 1):
            if g[i][j] != OPEN:
                continue
            if sum(g[i + dr][j + dc] == OPEN
                   for dr, dc in ((1, 0), (-1, 0), (0, 1), (0, -1))) == 1:
                deads.append((i, j))
    rng.shuffle(deads)
    for i, j in deads[: int(len(deads) * braid)]:
        opts = [(i + dr, j + dc)
                for dr, dc in ((1, 0), (-1, 0), (0, 1), (0, -1))
                if 1 <= i + dr < r - 1 and 1 <= j + dc < c - 1
                and g[i + dr][j + dc] == WALL]
        if opts:
            br, bc = rng.choice(opts)
            g[br][bc] = OPEN
    return g


def gen_rooms(r, c, rng, tries=None):
    """Chambers carved into solid rock, joined by L-shaped corridors."""
    g = blank(r, c)
    tries = tries or max(8, (r * c) // 90)
    rooms = []
    for _ in range(tries * 4):
        if len(rooms) >= tries:
            break
        h = rng.randint(3, max(4, r // 5))
        w = rng.randint(3, max(4, c // 5))
        top = rng.randint(1, max(1, r - h - 2))
        left = rng.randint(1, max(1, c - w - 2))
        if any(not (left + w + 1 < L or left > L + W + 1
                    or top + h + 1 < T or top > T + H + 1)
               for T, L, H, W in rooms):
            continue
        rooms.append((top, left, h, w))
        for i in range(top, top + h):
            for j in range(left, left + w):
                g[i][j] = OPEN
    for k in range(1, len(rooms)):
        t1, l1, h1, w1 = rooms[k - 1]
        t2, l2, h2, w2 = rooms[k]
        r1, c1 = t1 + h1 // 2, l1 + w1 // 2
        r2, c2 = t2 + h2 // 2, l2 + w2 // 2
        for j in range(min(c1, c2), max(c1, c2) + 1):
            g[r1][j] = OPEN
        for i in range(min(r1, r2), max(r1, r2) + 1):
            g[i][c2] = OPEN
    return g


def gen_caves(r, c, rng, fill=0.44, steps=5):
    """Cellular automata. Wide organic caverns."""
    g = [[OPEN if rng.random() > fill else WALL for _ in range(c)]
         for _ in range(r)]
    for i in range(r):
        g[i][0] = g[i][c - 1] = WALL
    for j in range(c):
        g[0][j] = g[r - 1][j] = WALL
    for _ in range(steps):
        nxt = [row[:] for row in g]
        for i in range(1, r - 1):
            for j in range(1, c - 1):
                walls = sum(g[i + dr][j + dc] == WALL
                            for dr in (-1, 0, 1) for dc in (-1, 0, 1)
                            if (dr, dc) != (0, 0))
                nxt[i][j] = WALL if walls > 4 else OPEN
        g = nxt
    return g


def gen_open(r, c, rng, obstacles=0.06):
    """Open field with a few scattered pillars. BFS frontier gets huge."""
    g = blank(r, c, OPEN)
    for i in range(r):
        g[i][0] = g[i][c - 1] = WALL
    for j in range(c):
        g[0][j] = g[r - 1][j] = WALL
    for _ in range(int(r * c * obstacles)):
        i, j = rng.randint(1, r - 2), rng.randint(1, c - 2)
        g[i][j] = WALL
    return g


def gen_corridor(r, c, rng):
    """A single snaking corridor: 1 cell wide, as long as the grid allows.
    Depth equals length, which is what breaks a recursive DFS."""
    g = blank(r, c)
    for i in range(1, r - 1, 2):
        for j in range(1, c - 1):
            g[i][j] = OPEN
        if i + 2 < r - 1:
            g[i + 1][c - 2 if (i // 2) % 2 == 0 else 1] = OPEN
    return g


def gen_sealed(r, c, rng):
    """A cave with the goal walled into its own pocket. NOPATH is the answer."""
    g = gen_caves(r, c, rng)
    # carve a sealed 3x3 room in the bottom-right and put the goal in it
    top, left = r - 6, c - 6
    for i in range(top, top + 5):
        for j in range(left, left + 5):
            g[i][j] = WALL
    for i in range(top + 1, top + 4):
        for j in range(left + 1, left + 4):
            g[i][j] = OPEN
    return g


def gen_comb(r, c, rng):
    """
    Adversarial for DFS: a spine along the top with long teeth hanging down.
    The goal sits at the end of the LAST tooth, so a DFS that scans its
    neighbours in the wrong order walks every tooth first.
    """
    g = blank(r, c)
    for j in range(1, c - 1):
        g[1][j] = OPEN
    for j in range(2, c - 1, 2):
        for i in range(1, r - 1):
            g[i][j] = OPEN
    return g


# ---------------------------------------------------------------- waypoints

def reachable_cells(g, src):
    r, c = len(g), len(g[0])
    seen = {src}
    q = deque([src])
    while q:
        i, j = q.popleft()
        for dr, dc in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            ni, nj = i + dr, j + dc
            if 0 <= ni < r and 0 <= nj < c and g[ni][nj] != WALL \
                    and (ni, nj) not in seen:
                seen.add((ni, nj))
                q.append((ni, nj))
    return seen


def bfs_dist(g, src):
    r, c = len(g), len(g[0])
    dist = {src: 0}
    q = deque([src])
    while q:
        i, j = q.popleft()
        for dr, dc in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            ni, nj = i + dr, j + dc
            if 0 <= ni < r and 0 <= nj < c and g[ni][nj] != WALL \
                    and (ni, nj) not in dist:
                dist[(ni, nj)] = dist[(i, j)] + 1
                q.append((ni, nj))
    return dist


def pick_waypoints(g, k, rng, style):
    """Waypoint 0 and 1 are chosen far apart; extras are spread out."""
    r, c = len(g), len(g[0])
    opens = [(i, j) for i in range(r) for j in range(c) if g[i][j] != WALL]
    if not opens:
        raise ValueError("no open cells")

    # largest connected component
    best = set()
    unseen = set(opens)
    while unseen:
        comp = reachable_cells(g, next(iter(unseen)))
        unseen -= comp
        if len(comp) > len(best):
            best = comp
    comp = sorted(best)

    if style == "comb":
        start = (1, 1)
        far = max((p for p in comp), key=lambda p: (p[1], p[0]))
        pts = [start, far]
    elif style == "sealed":
        pts = [comp[0], (r - 4, c - 4)]          # goal sits in the sealed pocket
        return pts[:k]
    elif style == "corridor":
        pts = [(1, 1), max(comp, key=lambda p: bfs_dist(g, (1, 1)).get(p, 0))]
    else:
        # double sweep for an approximate diameter
        d1 = bfs_dist(g, comp[0])
        a = max((p for p in comp if p in d1), key=lambda p: d1[p])
        d2 = bfs_dist(g, a)
        b = max((p for p in comp if p in d2), key=lambda p: d2[p])
        pts = [a, b]

    while len(pts) < k:
        # farthest-point sampling: maximise distance to the nearest chosen pt
        dists = [bfs_dist(g, p) for p in pts]
        cand = max(comp, key=lambda q: min(d.get(q, 0) for d in dists))
        if cand in pts:
            cand = rng.choice([p for p in comp if p not in pts])
        pts.append(cand)
    return pts[:k]


# ---------------------------------------------------------------- emit

GENERATORS = {
    "perfect": gen_perfect,
    "braid": gen_braid,
    "rooms": gen_rooms,
    "caves": gen_caves,
    "comb": gen_comb,
    "open": gen_open,
    "corridor": gen_corridor,
    "sealed": gen_sealed,
}


def make(style, size, seed, k=2, cols=None, terrain=False, tiers=None):
    rng = random.Random(seed)
    r = size
    c = cols or size
    if style in ("perfect", "braid", "comb"):
        r = r if r % 2 else r + 1
        c = c if c % 2 else c + 1
    g = GENERATORS[style](r, c, rng)
    if terrain:
        g = add_terrain(g, rng, tiers)
    pts = pick_waypoints(g, k, rng, style)
    return g, pts


def render(g, pts):
    r, c = len(g), len(g[0])
    out = [f"{r} {c}"]
    out += ["".join(row) for row in g]
    out.append(str(len(pts)))
    out += [f"{i} {j}" for i, j in pts]
    return "\n".join(out) + "\n"


def add_hazards(g, rng, fire_share=0.07, thorn_share=0.07):
    """
    No biomes. The floor stays uniform and cost comes from things placed ON
    it: small fires (9) and thorn patches (4), in clusters big enough to be
    worth routing around but small enough to cross when the detour is long.
    """
    r, c = len(g), len(g[0])
    open_cells = [(i, j) for i in range(r) for j in range(c) if g[i][j] == OPEN]
    if not open_cells:
        return g

    def scatter(glyph, share, lo, hi):
        target = int(len(open_cells) * share)
        placed = 0
        guard = 0
        while placed < target and guard < 20000:
            guard += 1
            si, sj = rng.choice(open_cells)
            if g[si][sj] != OPEN:
                continue
            size = rng.randint(lo, hi)
            front = [(si, sj)]
            while front and size > 0:
                i, j = front.pop(rng.randrange(len(front)))
                if not (0 <= i < r and 0 <= j < c) or g[i][j] != OPEN:
                    continue
                g[i][j] = glyph
                placed += 1
                size -= 1
                for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    front.append((i + di, j + dj))

    scatter("!", fire_share, 2, 7)      # fire, cost 9
    scatter("*", thorn_share, 3, 10)    # thorns, cost 4
    return g


def smooth_biomes(g, rounds=3):
    """Majority filter. Kills single-cell speckle so regions read as regions."""
    r, c = len(g), len(g[0])
    for _ in range(rounds):
        nxt = [row[:] for row in g]
        for i in range(1, r - 1):
            for j in range(1, c - 1):
                if g[i][j] == WALL:
                    continue
                tally = {}
                for di in (-1, 0, 1):
                    for dj in (-1, 0, 1):
                        ch = g[i + di][j + dj]
                        if ch == WALL:
                            continue
                        tally[ch] = tally.get(ch, 0) + (2 if (di or dj) else 1)
                if tally:
                    nxt[i][j] = max(tally, key=lambda k: (tally[k], k))
        g = nxt
    return g


# Cost tiers, written straight into the map as digits.
TIERS_DENSE = [("2", 0.22), ("4", 0.16), ("6", 0.16), ("9", 0.10)]
TIERS_SPARSE = [("3", 0.08), ("6", 0.10), ("9", 0.14)]


def add_terrain(g, rng, tiers=None):
    tiers = tiers or TIERS_DENSE
    """
    Grow cost patches so the cheapest route and the shortest route pull
    apart. Every tile stays positive: negative costs break Dijkstra, which
    is a bonus stage, not this one.
    """
    r, c = len(g), len(g[0])
    open_cells = [(i, j) for i in range(r) for j in range(c) if g[i][j] == OPEN]
    if not open_cells:
        return g
    for glyph, share in tiers:
        target = int(len(open_cells) * share)
        painted = 0
        guard = 0
        while painted < target and guard < 20000:
            guard += 1
            si, sj = rng.choice(open_cells)
            if g[si][sj] != OPEN:
                continue
            blob = max(4, int(target * rng.uniform(0.06, 0.20)))
            front = [(si, sj)]
            while front and blob > 0:
                i, j = front.pop(rng.randrange(len(front)))
                if not (0 <= i < r and 0 <= j < c) or g[i][j] != OPEN:
                    continue
                g[i][j] = glyph
                painted += 1
                blob -= 1
                for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    front.append((i + di, j + dj))
    return smooth_biomes(g)


SUITE = [
    # name              style      size  seed  k
    ("m01-perfect-21",  "perfect",   21,   11, 2),
    ("m02-perfect-41",  "perfect",   41,   23, 2),
    ("m03-braid-31",    "braid",     31,   31, 2),
    ("m04-rooms-35",    "rooms",     35,   47, 2),
    ("m05-caves-45",    "caves",     45,   59, 2),
    ("m06-comb-25",     "comb",      25,    3, 2),
    ("m07-perfect-81",  "perfect",   81,   71, 2),
    ("m08-caves-101",   "caves",    101,   89, 2),
    ("m09-open-61",     "open",      61,    7, 2),   # BFS frontier at its widest
]

# Stages 3-4. Seeds chosen so that on every one of these the cheapest path is
# STRICTLY LONGER in steps than the fewest-steps path, BFS pays at least 2x
# what Dijkstra pays, and A* expands well under Dijkstra.
# (t03 is the exception on the last point, deliberately: in a maze the
# heuristic has almost nothing to work with. Ask why.)
TERRAIN_SUITE = [
    # name              style      size  seed  k   tiers
    ("t01-rooms-35",    "rooms",     35,   49, 2, "sparse"),
    ("t02-caves-45",    "caves",     45,   51, 2, "dense"),
    ("t03-braid-31",    "braid",     31,   15, 2, "sparse"),
    ("t04-caves-81",    "caves",     81,   22, 2, "sparse"),
    ("t05-open-51",     "open",      51,   56, 2, "sparse"),
]

# Stages 5-6. K climbs to where (k-1)! actually hurts:
#   k=8 -> 5,040   k=10 -> 362,880   k=12 -> 39.9M   k=13 -> 479M
WAYPOINT_SUITE = [
    ("w01-rooms-31",    "rooms",     31,   17,  6),
    ("w02-caves-45",    "caves",     45,   23,  8),
    ("w03-braid-41",    "braid",     41,   37,  8),
    ("w04-caves-61",    "caves",     61,   43, 10),
    ("w05-caves-61",    "caves",     61,   77, 12),
    ("w06-rooms-71",    "rooms",     71,   91, 13),
    ("w07-caves-71",    "caves",     71,  103, 14),   # brute force: over a minute
]

# Edge cases. Small, and each one exists to break a specific wrong answer.
EDGE_SUITE = [
    ("e01-sealed-31",   "sealed",    31,    5, 2),   # goal unreachable: NOPATH
    ("e02-corridor-41", "corridor",  41,    1, 2),   # one long snake: depth == length
    ("e03-adjacent-11", "perfect",   11,    2, 2),   # start next to goal
    ("e04-same-11",     "perfect",   11,    2, 2),   # start == goal
    ("e05-tiny-3",      "perfect",    3,    1, 2),   # 3x3: a single open cell
]


def build_suite(outdir, json_path=None):
    import json as _json
    os.makedirs(outdir, exist_ok=True)
    blob = {}
    entries = []
    for e in SUITE + WAYPOINT_SUITE + EDGE_SUITE:
        entries.append(e + ("dense",))
    for e in TERRAIN_SUITE:
        entries.append(e)
    for name, style, size, seed, k, tiers in entries:
        terrain = name.startswith(("t", "w"))
        tset = TIERS_SPARSE if tiers == "sparse" else TIERS_DENSE
        g, pts = make(style, size, seed, k, terrain=terrain, tiers=tset)
        if name.startswith("e03"):      # goal one step from start
            s0 = pts[0]
            for di, dj in ((0, 1), (1, 0), (0, -1), (-1, 0)):
                if g[s0[0] + di][s0[1] + dj] != WALL:
                    pts = [s0, (s0[0] + di, s0[1] + dj)]
                    break
        if name.startswith("e04"):
            pts = [pts[0], pts[0]]
        if name.startswith("e05"):
            pts = [(1, 1), (1, 1)]
        with open(os.path.join(outdir, name + ".grid"), "w") as f:
            f.write(render(g, pts))
        opens = sum(1 for row in g for ch in row if ch != WALL)
        blob[name] = {"style": style, "rows": len(g), "cols": len(g[0]),
                      "open": opens, "grid": ["".join(row) for row in g],
                      "waypoints": [list(p) for p in pts]}
        print(f"{name:18s} {len(g):>4}x{len(g[0]):<4} open={opens:<6} k={k}")
    if json_path:
        with open(json_path, "w") as f:
            _json.dump(blob, f, separators=(",", ":"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--style", choices=sorted(GENERATORS))
    ap.add_argument("--size", type=int, default=31)
    ap.add_argument("--cols", type=int)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--terrain", action="store_true",
                    help="sprinkle cost patches (stages 3+)")
    ap.add_argument("--waypoints", type=int, default=2)
    ap.add_argument("--out")
    ap.add_argument("--suite", action="store_true")
    ap.add_argument("--outdir", default="mazes")
    ap.add_argument("--json", help="also write the suite as a JS/JSON blob")
    a = ap.parse_args()

    if a.suite:
        build_suite(a.outdir, a.json)
        return

    if not a.style:
        ap.error("--style required unless --suite")
    g, pts = make(a.style, a.size, a.seed, a.waypoints, a.cols, a.terrain)
    text = render(g, pts)
    if a.out:
        with open(a.out, "w") as f:
            f.write(text)
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
