#!/usr/bin/env python3
"""
bench.py -- runs every measurement Phase 2 needs and writes bench/results.csv.

    ./bench                 everything (brute force up to k14: a couple of minutes)
    ./bench --quick         skip k13 and k14
    ./bench --reps 11       more repeats for the timings (default 7, median taken)

Timings are wall-clock microseconds, taken as the median of the repeats, so a
single slow run does not distort a point. Everything else is a count the
program reports and is identical on every machine.

Columns:
  map, N, V, K, algorithm, expanded, max_frontier, cost, steps,
  time_us, peak_kb, candidates
"""
import argparse, csv, re, statistics, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BENCH = ROOT / 'bench'
S_MAPS = ['s11','s15','s21','s25','s31','s41']
K_MAPS = [f'k{k:02d}' for k in range(4, 15)]

def build():
    r = subprocess.run(['make','-s','bin/dfs','bin/bfs','bin/dijkstra','bin/astar','bin/floyd','bin/tsp'],
                       cwd=ROOT, capture_output=True, text=True)
    if r.returncode: sys.exit('build failed:\n' + r.stdout + r.stderr)

def run(args):
    out = subprocess.run(args, cwd=ROOT, capture_output=True, text=True).stdout
    return dict(re.findall(r'^(\w+) (\S+)$', out, re.M))

def dims(path):
    rows = path.read_text().splitlines()
    R, C = map(int, rows[0].split())
    N = sum(ch != '#' for r in rows[1:1+R] for ch in r)
    K = int(rows[1+R])
    return N, R*C, K

def median_time(args, key, reps):
    ts = []
    for _ in range(reps):
        o = run(args); ts.append(int(o[key]))
    return int(statistics.median(ts)), o

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--quick', action='store_true'); ap.add_argument('--reps', type=int, default=7)
    a = ap.parse_args()
    build()
    rows = []
    def add(**kw):
        base = dict(map='', N='', V='', K='', algorithm='', expanded='', max_frontier='',
                    cost='', steps='', time_us='', peak_kb='', candidates='')
        base.update(kw); rows.append(base)

    for m in S_MAPS:
        f = BENCH / f'{m}.grid'; N, V, K = dims(f)
        for alg in ['dfs','bfs','dijkstra','astar']:
            t, o = median_time([f'./bin/{alg}','--quiet',str(f)], 'TIME_US', a.reps)
            add(map=m, N=N, V=V, K=K, algorithm=alg, expanded=o['EXPANDED'], max_frontier=o['MAXFRONTIER'],
                cost=o.get('COST',''), steps=(int(o['LEN'])-1) if 'LEN' in o else '', time_us=t)
            print(f'  {m} {alg:9s} expanded={o["EXPANDED"]:>5} maxFrontier={o["MAXFRONTIER"]:>5} time={t} us', flush=True)
        for mode, name in [('--fw','floyd-warshall'), ('--dij','repeated-dijkstra')]:
            t, o = median_time(['./bin/floyd', mode, '--quiet', str(f)], 'TIME_US', a.reps)
            add(map=m, N=N, V=V, K=K, algorithm=name, time_us=t, peak_kb=o['PEAK_KB'])
            print(f'  {m} {name:18s} time={t} us peak={o["PEAK_KB"]} KB', flush=True)

    for m in ['m02-perfect-41']:
        f = ROOT / 'maps' / f'{m}.grid'; N, V, K = dims(f)
        for alg in ['dfs','bfs']:
            o = run([f'./bin/{alg}','--quiet',str(f)])
            add(map=m, N=N, V=V, K=K, algorithm=alg, expanded=o['EXPANDED'], max_frontier=o['MAXFRONTIER'],
                cost=o.get('COST',''), steps=(int(o['LEN'])-1) if 'LEN' in o else '')
            print(f'  {m} {alg:9s} maxFrontier={o["MAXFRONTIER"]}', flush=True)

    for m in K_MAPS:
        K = int(m[1:])
        if a.quick and K >= 13: continue
        f = BENCH / f'{m}.grid'; N, V, _ = dims(f)
        reps = a.reps if K <= 10 else (3 if K <= 12 else 1)
        t, o = median_time(['./bin/tsp','--quiet',str(f)], 'TOUR_US', reps)
        add(map=m, N=N, V=V, K=K, algorithm='brute-force', cost=o['COST'], time_us=t, candidates=o['CANDIDATES'])
        print(f'  {m} brute-force  orders={int(o["CANDIDATES"]):>13,}  time={t/1e6:9.3f} s', flush=True)

    out = BENCH / 'results.csv'
    with open(out, 'w', newline='') as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)
    print(f'\nwrote {out} ({len(rows)} rows)')

if __name__ == '__main__':
    main()
