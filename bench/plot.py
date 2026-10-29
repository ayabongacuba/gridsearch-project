#!/usr/bin/env python3
"""
plot.py -- draws the five Phase 2 plots from bench/results.csv.

    python3 bench/plot.py            writes bench/plots/plot1.png ... plot5.png

This is a starting point, not the submission. It draws your measurements; you
add your predictions to each plot, and you write the text that goes with it.
Edit freely.

Needs matplotlib:  pip install matplotlib
"""
import csv, sys
from collections import defaultdict
from pathlib import Path
try:
    import matplotlib; matplotlib.use('Agg'); import matplotlib.pyplot as plt
except ImportError:
    sys.exit('pip install matplotlib')

HERE = Path(__file__).resolve().parent
rows = list(csv.DictReader(open(HERE / 'results.csv')))
out = HERE / 'plots'; out.mkdir(exist_ok=True)
COL = {'dfs':'#E2553A','bfs':'#C7951B','dijkstra':'#7C5CE0','astar':'#0F9F8C',
       'floyd-warshall':'#0284C7','repeated-dijkstra':'#7C5CE0','brute-force':'#E2553A'}
NAME = {'dfs':'Depth-first','bfs':'Breadth-first','dijkstra':'Dijkstra','astar':'A*',
        'floyd-warshall':'Floyd-Warshall','repeated-dijkstra':'Repeated Dijkstra','brute-force':'Brute force'}

def series(alg, x, y, where=lambda r: r['map'].startswith('s')):
    pts = sorted((int(r[x]), float(r[y])) for r in rows if r['algorithm']==alg and where(r) and r[y] != '')
    return [p[0] for p in pts], [p[1] for p in pts]

def finish(ax, xl, yl):
    ax.set_xlabel(xl); ax.set_ylabel(yl); ax.grid(alpha=.3); ax.legend(frameon=False)
    for s in ('top','right'): ax.spines[s].set_visible(False)

# 1. how much each search holds at once
fig, ax = plt.subplots(figsize=(7,4.2))
for alg in ['dfs','bfs']:
    x, y = series(alg, 'N', 'max_frontier'); ax.plot(x, y, '-o', color=COL[alg], label=NAME[alg])
finish(ax, 'N, open cells', 'maxFrontier'); ax.set_title('Plot 1 - the most cells held at once')
fig.tight_layout(); fig.savefig(out / 'plot1.png', dpi=150); plt.close(fig)

# 2. time
fig, (a1, a2) = plt.subplots(1, 2, figsize=(10,4.2))
for alg in ['dfs','bfs','dijkstra','astar']:
    x, y = series(alg, 'N', 'time_us'); a1.plot(x, y, '-o', color=COL[alg], label=NAME[alg])
finish(a1, 'N, open cells', 'microseconds'); a1.set_title('Plot 2a - search time')
for alg in ['floyd-warshall','repeated-dijkstra']:
    x, y = series(alg, 'V', 'time_us'); a2.plot(x, [v/1000 for v in y], '-o', color=COL[alg], label=NAME[alg])
finish(a2, 'V, all cells', 'milliseconds'); a2.set_title('Plot 2b - building the table of costs')
fig.tight_layout(); fig.savefig(out / 'plot2.png', dpi=150); plt.close(fig)

# 3. A* against Dijkstra
fig, ax = plt.subplots(figsize=(7,4.2))
for alg in ['dijkstra','astar']:
    x, y = series(alg, 'N', 'expanded'); ax.plot(x, y, '-o', color=COL[alg], label=NAME[alg])
finish(ax, 'N, open cells', 'cells committed to'); ax.set_title('Plot 3 - what the estimate saves')
fig.tight_layout(); fig.savefig(out / 'plot3.png', dpi=150); plt.close(fig)

# 4. route cost
fig, ax = plt.subplots(figsize=(7,4.2))
for alg in ['dfs','bfs','dijkstra','astar']:
    x, y = series(alg, 'N', 'cost'); ax.plot(x, y, '-o', color=COL[alg], label=NAME[alg])
finish(ax, 'N, open cells', 'route cost'); ax.set_title('Plot 4 - the cost of the route each returns')
fig.tight_layout(); fig.savefig(out / 'plot4.png', dpi=150); plt.close(fig)

# 5. brute force
fig, (a1, a2) = plt.subplots(1, 2, figsize=(10,4.2))
x, y = series('brute-force', 'K', 'time_us', where=lambda r: r['map'].startswith('k'))
a1.plot(x, [v/1e6 for v in y], '-o', color=COL['brute-force'], label=NAME['brute-force']); a1.axhline(60, ls=':', color='#888')
finish(a1, 'K, stops', 'seconds'); a1.set_title('Plot 5a - brute force, all of it')
small = [(k, v) for k, v in zip(x, y) if k <= 11]
a2.plot([k for k,_ in small], [v/1e6 for _,v in small], '-o', color=COL['brute-force'], label=NAME['brute-force'])
finish(a2, 'K, stops', 'seconds'); a2.set_title('Plot 5b - k04 to k11 on their own scale')
fig.tight_layout(); fig.savefig(out / 'plot5.png', dpi=150); plt.close(fig)
print('wrote', *sorted(p.name for p in out.glob('plot*.png')))
