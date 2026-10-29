// window_sdl.cpp -- the SDL2 half of window.h.
//
// Compiled into every stage binary. When SDL2 is not available the whole
// file collapses to a stub that reports failure, and the driver falls back
// to writing an HTML replay.
//
// Provided. You should not need to change this file.

#include "window.h"

#ifndef GS_HAVE_SDL

#include <vector>
namespace gs { namespace window {
bool replay(const std::string &, const std::string &) { return false; }
bool replayGrid(const std::vector<std::string> &, const std::string &) { return false; }
}}  // namespace gs::window

#else

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace gs {
namespace window {
namespace {

// ---------------------------------------------------------------- palette
struct Col { Uint8 r, g, b; };
constexpr Col WALL{0x18, 0x1D, 0x3B};
constexpr Col VOID{0x04, 0x05, 0x0A};
constexpr Col CHROME{0x0B, 0x0D, 0x18};
constexpr Col RULE{0x23, 0x2A, 0x4A};
constexpr Col LEAD{0x7C, 0xC4, 0xFF};   // frontier, ice blue
constexpr Col GRASS{0x22, 0xC5, 0x5E};  // the path it found
constexpr Col CHALK{0xE8, 0xDC, 0xC4};
constexpr Col MUTED{0x7C, 0x86, 0xA8};
constexpr Col LAMP{0xF0, 0xA9, 0x3B};
constexpr Col COOL{0x7A, 0x3D, 0x1C};

Col ramp(double t) {                      // cooled -> lamp
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return Col{(Uint8)(COOL.r + (LAMP.r - COOL.r) * t),
               (Uint8)(COOL.g + (LAMP.g - COOL.g) * t),
               (Uint8)(COOL.b + (LAMP.b - COOL.b) * t)};
}

// ---------------------------------------------------------------- 3x5 font
// Five rows of three bits per glyph. Enough for the readout, nothing more.
struct Glyph { unsigned char row[5]; };
Glyph glyphFor(char c) {
    switch (c) {
        case '0': return {{0b111,0b101,0b101,0b101,0b111}};
        case '1': return {{0b010,0b110,0b010,0b010,0b111}};
        case '2': return {{0b111,0b001,0b111,0b100,0b111}};
        case '3': return {{0b111,0b001,0b111,0b001,0b111}};
        case '4': return {{0b101,0b101,0b111,0b001,0b001}};
        case '5': return {{0b111,0b100,0b111,0b001,0b111}};
        case '6': return {{0b111,0b100,0b111,0b101,0b111}};
        case '7': return {{0b111,0b001,0b010,0b010,0b010}};
        case '8': return {{0b111,0b101,0b111,0b101,0b111}};
        case '9': return {{0b111,0b101,0b111,0b001,0b111}};
        case 'A': return {{0b111,0b101,0b111,0b101,0b101}};
        case 'B': return {{0b110,0b101,0b110,0b101,0b110}};
        case 'C': return {{0b111,0b100,0b100,0b100,0b111}};
        case 'D': return {{0b110,0b101,0b101,0b101,0b110}};
        case 'E': return {{0b111,0b100,0b111,0b100,0b111}};
        case 'F': return {{0b111,0b100,0b111,0b100,0b100}};
        case 'G': return {{0b111,0b100,0b101,0b101,0b111}};
        case 'H': return {{0b101,0b101,0b111,0b101,0b101}};
        case 'I': return {{0b111,0b010,0b010,0b010,0b111}};
        case 'J': return {{0b001,0b001,0b001,0b101,0b111}};
        case 'K': return {{0b101,0b101,0b110,0b101,0b101}};
        case 'L': return {{0b100,0b100,0b100,0b100,0b111}};
        case 'M': return {{0b101,0b111,0b111,0b101,0b101}};
        case 'N': return {{0b101,0b111,0b111,0b111,0b101}};
        case 'O': return {{0b111,0b101,0b101,0b101,0b111}};
        case 'P': return {{0b111,0b101,0b111,0b100,0b100}};
        case 'Q': return {{0b111,0b101,0b101,0b111,0b011}};
        case 'R': return {{0b111,0b101,0b110,0b101,0b101}};
        case 'S': return {{0b111,0b100,0b111,0b001,0b111}};
        case 'T': return {{0b111,0b010,0b010,0b010,0b010}};
        case 'U': return {{0b101,0b101,0b101,0b101,0b111}};
        case 'V': return {{0b101,0b101,0b101,0b101,0b010}};
        case 'W': return {{0b101,0b101,0b111,0b111,0b101}};
        case 'X': return {{0b101,0b101,0b010,0b101,0b101}};
        case 'Y': return {{0b101,0b101,0b010,0b010,0b010}};
        case 'Z': return {{0b111,0b001,0b010,0b100,0b111}};
        case ':': return {{0b000,0b010,0b000,0b010,0b000}};
        case '/': return {{0b001,0b001,0b010,0b100,0b100}};
        case '-': return {{0b000,0b000,0b111,0b000,0b000}};
        case '.': return {{0b000,0b000,0b000,0b000,0b010}};
        case ',': return {{0b000,0b000,0b000,0b010,0b100}};
        case '+': return {{0b000,0b010,0b111,0b010,0b000}};
        default:  return {{0,0,0,0,0}};
    }
}

void text(SDL_Renderer *R, int x, int y, const std::string &s, Col c, int px) {
    SDL_SetRenderDrawColor(R, c.r, c.g, c.b, 255);
    int cx = x;
    for (char ch : s) {
        if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
        Glyph gl = glyphFor(ch);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (gl.row[row] & (1 << (2 - col))) {
                    SDL_Rect r{cx + col * px, y + row * px, px, px};
                    SDL_RenderFillRect(R, &r);
                }
        cx += 4 * px;
    }
}

std::string commas(long long v) {
    std::string s = std::to_string(v), out;
    int n = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.push_back(s[i]);
        if (++n % 3 == 0 && i > 0) out.push_back(',');
    }
    std::reverse(out.begin(), out.end());
    return out;
}

// ---------------------------------------------------------------- trace
struct Ev { int t, r, c, p; };            // t: 1 expand, 2 push, 3 discard, 4 relax

struct Trace {
    int R = 0, C = 0;
    std::vector<std::string> map;
    int sr = 0, sc = 0, gr = 0, gc = 0;
    std::string algo = "search", inst;
    std::vector<Ev> ev;
    std::vector<int> path;
    long total = 0;
};

bool parse(const std::string &text, Trace &T) {
    std::istringstream in(text);
    std::string line;
    bool events = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (events) {
            if (line.empty()) continue;
            char k = line[0];
            int a, b, c, d;
            if (k == 'E' && std::sscanf(line.c_str() + 1, "%d %d %d %d", &a, &b, &c, &d) == 4) {
                T.ev.push_back({1, a, b, c < 0 ? -1 : c * T.C + d});
                ++T.total;
            } else if (k == 'P' && std::sscanf(line.c_str() + 1, "%d %d", &a, &b) == 2) {
                T.ev.push_back({2, a, b, -1});
            } else if (k == 'D' && std::sscanf(line.c_str() + 1, "%d %d", &a, &b) == 2) {
                T.ev.push_back({3, a, b, -1});
            } else if (k == 'R') {
                int rr, cc, gg, pr, pc;
                if (std::sscanf(line.c_str() + 1, "%d %d %d %d %d",
                                &rr, &cc, &gg, &pr, &pc) == 5)
                    T.ev.push_back({4, rr, cc, pr * T.C + pc});
            } else if (k == 'W' && std::sscanf(line.c_str() + 1, "%d %d", &a, &b) == 2) {
                T.path.push_back(a * T.C + b);
            }
            continue;
        }
        if (line.rfind("ALGORITHM ", 0) == 0) T.algo = line.substr(10);
        else if (line.rfind("INSTANCE ", 0) == 0) T.inst = line.substr(9);
        else if (line.rfind("GRID ", 0) == 0) std::sscanf(line.c_str() + 5, "%d %d", &T.R, &T.C);
        else if (line == "MAP") {
            for (int i = 0; i < T.R; ++i) { std::getline(in, line);
                if (!line.empty() && line.back() == '\r') line.pop_back();
                T.map.push_back(line); }
        }
        else if (line.rfind("START ", 0) == 0) std::sscanf(line.c_str() + 6, "%d %d", &T.sr, &T.sc);
        else if (line.rfind("GOAL ", 0) == 0) std::sscanf(line.c_str() + 5, "%d %d", &T.gr, &T.gc);
        else if (line == "EVENTS") events = true;
    }
    return T.R > 0 && (int)T.map.size() == T.R;
}

// ---------------------------------------------------------------- state
struct State {
    std::vector<char> visited;
    std::vector<int> parent, when, pend;
    long expanded = 0, frontier = 0, maxFrontier = 0;
    int head = -1;
    size_t cursor = 0;
};

void reset(State &S, int n) {
    S.visited.assign(n, 0);
    S.parent.assign(n, -1);
    S.when.assign(n, -1);
    S.pend.assign(n, 0);
    S.expanded = S.frontier = S.maxFrontier = 0;
    S.head = -1;
    S.cursor = 0;
}

void step(State &S, const Trace &T) {
    const Ev &e = T.ev[S.cursor++];
    int n = e.r * T.C + e.c;
    if (e.t == 2)      { S.pend[n]++; S.frontier++; }
    else if (e.t == 3) { S.pend[n]--; S.frontier--; }
    else if (e.t == 4) { S.pend[n]++; S.frontier++; S.parent[n] = e.p; }
    else {
        S.pend[n]--; S.frontier--;
        S.visited[n] = 1; S.parent[n] = e.p; S.when[n] = (int)S.expanded;
        S.head = n; S.expanded++;
    }
    if (S.frontier > S.maxFrontier) S.maxFrontier = S.frontier;
}

void seek(State &S, const Trace &T, size_t target) {
    if (target < S.cursor) reset(S, T.R * T.C);
    while (S.cursor < target && S.cursor < T.ev.size()) step(S, T);
}

// Draws one map into `area`. Shared by the single view and the 2x2 grid.
void drawPanel(SDL_Renderer *R, const Trace &T, const State &S, SDL_Rect area,
               long window, const char *caption) {
    const bool settled = (S.expanded >= T.total) && !T.path.empty();
    const int pad = caption ? 22 : 14;
    const int cs = std::max(1, std::min((area.w - pad * 2) / T.C,
                                        (area.h - pad * 2 - (caption ? 14 : 0)) / T.R));
    const int ox = area.x + (area.w - cs * T.C) / 2;
    const int oy = area.y + (caption ? 18 : 0) + (area.h - (caption ? 18 : 0) - cs * T.R) / 2;
    auto cx = [&](int n) { return ox + (n % T.C) * cs + cs / 2; };
    auto cy = [&](int n) { return oy + (n / T.C) * cs + cs / 2; };

    // Walls, then any corridor dearer than cost 1 gets a faint mauve lift so
    // the terrain a route is avoiding is visible underneath the search.
    for (int r = 0; r < T.R; ++r)
        for (int c = 0; c < T.C; ++c) {
            char ch = T.map[r][c];
            SDL_Rect q{ox + c * cs, oy + r * cs, cs, cs};
            if (ch == '#') {
                SDL_SetRenderDrawColor(R, WALL.r, WALL.g, WALL.b, 255);
                SDL_RenderFillRect(R, &q);
            } else if (ch >= '2' && ch <= '9') {
                double t = (ch - '1') / 8.0;
                SDL_SetRenderDrawColor(R, (Uint8)(4 + 47 * t), (Uint8)(5 + 35 * t),
                                       (Uint8)(10 + 50 * t), 255);
                SDL_RenderFillRect(R, &q);
            }
        }

    // Every move is axis aligned, so each trail segment is just a rectangle
    // spanning two cell centres.
    const int th = std::max(1, (int)(cs * 0.56));
    for (int n = 0; n < (int)S.when.size(); ++n) {
        if (S.when[n] < 0) continue;
        long age = S.expanded - S.when[n];
        double t = age >= window ? 0.0 : 1.0 - (double)age / window;
        Col col = ramp(t);
        SDL_SetRenderDrawColor(R, col.r, col.g, col.b,
                               (Uint8)((128 + 127 * t) * (settled ? 0.5 : 1.0)));
        int x2 = cx(n), y2 = cy(n), p = S.parent[n];
        if (p >= 0) {
            int x1 = cx(p), y1 = cy(p);
            SDL_Rect q{std::min(x1, x2) - th / 2, std::min(y1, y2) - th / 2,
                       std::abs(x2 - x1) + th, std::abs(y2 - y1) + th};
            SDL_RenderFillRect(R, &q);
        } else {
            SDL_Rect q{x2 - th / 2, y2 - th / 2, th, th};
            SDL_RenderFillRect(R, &q);
        }
    }

    // Whole cells, so neighbouring frontier cells merge into one band.
    SDL_SetRenderDrawColor(R, LEAD.r, LEAD.g, LEAD.b, 255);
    for (int n = 0; n < (int)S.pend.size(); ++n) {
        if (S.pend[n] <= 0 || S.visited[n]) continue;
        SDL_Rect q{ox + (n % T.C) * cs, oy + (n / T.C) * cs, cs, cs};
        SDL_RenderFillRect(R, &q);
    }

    if (S.head >= 0 && !settled) {
        SDL_SetRenderDrawColor(R, CHALK.r, CHALK.g, CHALK.b, 190);
        const int rt = std::max(1, (int)(cs * 0.22));
        int n = S.head, guard = 0;
        while (S.parent[n] >= 0 && guard++ < T.R * T.C) {
            int p = S.parent[n];
            int x1 = cx(n), y1 = cy(n), x2 = cx(p), y2 = cy(p);
            SDL_Rect q{std::min(x1, x2) - rt / 2, std::min(y1, y2) - rt / 2,
                       std::abs(x2 - x1) + rt, std::abs(y2 - y1) + rt};
            SDL_RenderFillRect(R, &q);
            n = p;
        }
        SDL_SetRenderDrawColor(R, 255, 243, 214, 255);
        int hs = std::max(3, (int)(cs * 0.7));
        SDL_Rect q{cx(S.head) - hs / 2, cy(S.head) - hs / 2, hs, hs};
        SDL_RenderFillRect(R, &q);
    }

    if (settled) {
        // dark backing, then the route, so it stays legible over the trail
        auto stroke = [&](int wdt, Col col, Uint8 a) {
            SDL_SetRenderDrawColor(R, col.r, col.g, col.b, a);
            for (size_t i = 0; i + 1 < T.path.size(); ++i) {
                int x1 = cx(T.path[i]), y1 = cy(T.path[i]);
                int x2 = cx(T.path[i + 1]), y2 = cy(T.path[i + 1]);
                SDL_Rect q{std::min(x1, x2) - wdt / 2, std::min(y1, y2) - wdt / 2,
                           std::abs(x2 - x1) + wdt, std::abs(y2 - y1) + wdt};
                SDL_RenderFillRect(R, &q);
            }
        };
        stroke(std::max(3, (int)(cs * 0.74)), Col{0x04, 0x05, 0x0A}, 215);
        stroke(std::max(2, (int)(cs * 0.52)), GRASS, 255);
    }

    // Filled discs, approximated with a square and a dark core: thin outlines
    // disappear once the trail is dense around them.
    auto marker = [&](int r, int c, Col col) {
        int w = std::max(7, (int)(cs * 1.5));
        int mx = ox + c * cs + cs / 2, my = oy + r * cs + cs / 2;
        SDL_SetRenderDrawColor(R, col.r, col.g, col.b, 255);
        SDL_Rect q{mx - w / 2, my - w / 2, w, w};
        SDL_RenderFillRect(R, &q);
        SDL_SetRenderDrawColor(R, VOID.r, VOID.g, VOID.b, 255);
        int i2 = std::max(2, w / 3);
        SDL_Rect q2{mx - i2 / 2, my - i2 / 2, i2, i2};
        SDL_RenderFillRect(R, &q2);
    };
    marker(T.sr, T.sc, LEAD);
    marker(T.gr, T.gc, LAMP);

    if (caption) {
        text(R, area.x + 12, area.y + 7, caption, CHALK, 2);
        std::string right = (S.expanded >= T.total)
            ? "DONE AT " + commas(T.total)
            : "FRONTIER " + commas(S.frontier);
        Col rc = (S.expanded >= T.total) ? LEAD : MUTED;
        text(R, area.x + area.w - 12 - (int)right.size() * 8, area.y + 7, right, rc, 2);
    }
}

// Advance until this run has expanded `target` cells, or has run out.
void seekExpanded(State &S, const Trace &T, long target) {
    if (S.expanded > target) reset(S, T.R * T.C);
    while (S.expanded < target && S.cursor < T.ev.size()) step(S, T);
}

}  // namespace

// ---------------------------------------------------------------- replay
bool replay(const std::string &traceText, const std::string &title) {
    Trace T;
    if (!parse(traceText, T)) return false;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "window: SDL_Init failed (%s)\n", SDL_GetError());
        return false;
    }

    const int BAR = 46;
    int winW = 1100, winH = 900;
    SDL_Window *win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_RESIZABLE);
    if (!win) { SDL_Quit(); return false; }
    SDL_Renderer *R = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!R) R = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!R) { SDL_DestroyWindow(win); SDL_Quit(); return false; }
    SDL_SetRenderDrawBlendMode(R, SDL_BLENDMODE_BLEND);

    State S; reset(S, T.R * T.C);
    const double SPEED[5] = {60, 120, 260, 700, 1e9};
    int rate = 2;
    bool playing = true, running = true, dragging = false;
    double acc = 0;
    Uint64 last = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();
    const long window = std::min(400L, std::max(60L, T.total / 8));

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: case SDLK_q: running = false; break;
                    case SDLK_SPACE:
                        if (S.cursor >= T.ev.size()) reset(S, T.R * T.C);
                        playing = !playing; break;
                    case SDLK_RIGHT:
                        playing = false;
                        if (S.cursor < T.ev.size()) step(S, T);
                        break;
                    case SDLK_LEFT:
                        playing = false;
                        seek(S, T, S.cursor ? S.cursor - 1 : 0);
                        break;
                    case SDLK_PERIOD:
                        playing = false;
                        seek(S, T, std::min(T.ev.size(), S.cursor + 100));
                        break;
                    case SDLK_COMMA:
                        playing = false;
                        seek(S, T, S.cursor > 100 ? S.cursor - 100 : 0);
                        break;
                    case SDLK_r: reset(S, T.R * T.C); playing = true; break;
                    case SDLK_1: rate = 0; break;
                    case SDLK_2: rate = 1; break;
                    case SDLK_3: rate = 2; break;
                    case SDLK_4: rate = 3; break;
                    case SDLK_5: rate = 4; break;
                    default: break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION) {
                SDL_GetWindowSize(win, &winW, &winH);
                bool down = (e.type == SDL_MOUSEBUTTONDOWN);
                int my = down ? e.button.y : e.motion.y;
                int mx = down ? e.button.x : e.motion.x;
                if (down && my > winH - BAR) dragging = true;
                if (e.type == SDL_MOUSEMOTION && !(e.motion.state & SDL_BUTTON_LMASK))
                    dragging = false;
                if (dragging) {
                    playing = false;
                    double f = (double)(mx - 16) / std::max(1, winW - 32);
                    f = f < 0 ? 0 : f > 1 ? 1 : f;
                    seek(S, T, (size_t)(f * T.ev.size()));
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) dragging = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = std::min(0.1, (double)(now - last) / freq);
        last = now;
        if (playing) {
            acc += dt * SPEED[rate];
            long n = (long)acc; acc -= n;
            for (long i = 0; i < n && S.cursor < T.ev.size(); ++i) step(S, T);
            if (S.cursor >= T.ev.size()) playing = false;
        }

        SDL_GetWindowSize(win, &winW, &winH);
        const int mapH = winH - BAR;
        SDL_SetRenderDrawColor(R, VOID.r, VOID.g, VOID.b, 255);
        SDL_RenderClear(R);

        drawPanel(R, T, S, SDL_Rect{0, 0, winW, mapH}, window, nullptr);

        // ---- chrome ----
        SDL_SetRenderDrawColor(R, CHROME.r, CHROME.g, CHROME.b, 255);
        SDL_Rect bar{0, winH - BAR, winW, BAR};
        SDL_RenderFillRect(R, &bar);
        SDL_SetRenderDrawColor(R, RULE.r, RULE.g, RULE.b, 255);
        SDL_RenderDrawLine(R, 0, winH - BAR, winW, winH - BAR);

        SDL_Rect track{16, winH - BAR + 10, winW - 32, 3};
        SDL_SetRenderDrawColor(R, RULE.r, RULE.g, RULE.b, 255);
        SDL_RenderFillRect(R, &track);
        double frac = T.ev.empty() ? 0 : (double)S.cursor / T.ev.size();
        SDL_SetRenderDrawColor(R, LAMP.r, LAMP.g, LAMP.b, 255);
        SDL_Rect fill{16, winH - BAR + 10, (int)((winW - 32) * frac), 3};
        SDL_RenderFillRect(R, &fill);
        SDL_Rect knob{16 + (int)((winW - 32) * frac) - 4, winH - BAR + 6, 9, 11};
        SDL_RenderFillRect(R, &knob);

        std::string head = T.algo + "  " + T.inst;
        text(R, 16, 12, head, LAMP, 3);

        std::string line1 = "EXPANDED " + commas(S.expanded) +
                            "   FRONTIER " + commas(S.frontier) +
                            "   MAX " + commas(S.maxFrontier);
        text(R, 16, winH - BAR + 24, line1, CHALK, 2);
        const char *rl[5] = {"1X", "2X", "4X", "8X", "MAX"};
        std::string right = std::string(playing ? "PLAYING " : "PAUSED  ") + rl[rate];
        text(R, winW - 16 - (int)right.size() * 8, winH - BAR + 24, right, MUTED, 2);

        SDL_RenderPresent(R);
        if (!playing) SDL_Delay(12);
    }

    SDL_DestroyRenderer(R);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return true;
}

bool replayGrid(const std::vector<std::string> &traces, const std::string &title) {
    std::vector<Trace> T(traces.size());
    std::vector<State> S(traces.size());
    long maxExp = 0;
    for (size_t i = 0; i < traces.size(); ++i) {
        if (!parse(traces[i], T[i])) return false;
        reset(S[i], T[i].R * T[i].C);
        maxExp = std::max(maxExp, T[i].total);
    }
    if (T.empty()) return false;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;

    const int BAR = 46;
    int winW = 1400, winH = 1000;
    SDL_Window *win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_RESIZABLE);
    if (!win) { SDL_Quit(); return false; }
    SDL_Renderer *R = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!R) R = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if (!R) { SDL_DestroyWindow(win); SDL_Quit(); return false; }
    SDL_SetRenderDrawBlendMode(R, SDL_BLENDMODE_BLEND);

    // 1 -> 1x1, 2 -> 2x1, 3..4 -> 2x2, 5..6 -> 3x2
    const int n = (int)T.size();
    const int cols = n <= 1 ? 1 : n <= 4 ? 2 : 3;
    const int rows = (n + cols - 1) / cols;

    const double SPEED[5] = {60, 120, 260, 700, 1e9};
    int rate = 2;
    bool playing = true, running = true, dragging = false;
    long at = 0;
    double acc = 0;
    Uint64 last = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    auto go = [&](long target) {
        at = std::max(0L, std::min(maxExp, target));
        for (size_t i = 0; i < T.size(); ++i) seekExpanded(S[i], T[i], at);
    };

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: case SDLK_q: running = false; break;
                    case SDLK_SPACE:
                        if (at >= maxExp) go(0);
                        playing = !playing; break;
                    case SDLK_RIGHT: playing = false; go(at + 1); break;
                    case SDLK_LEFT:  playing = false; go(at - 1); break;
                    case SDLK_PERIOD: playing = false; go(at + 100); break;
                    case SDLK_COMMA:  playing = false; go(at - 100); break;
                    case SDLK_r: go(0); playing = true; break;
                    case SDLK_1: rate = 0; break;
                    case SDLK_2: rate = 1; break;
                    case SDLK_3: rate = 2; break;
                    case SDLK_4: rate = 3; break;
                    case SDLK_5: rate = 4; break;
                    default: break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION) {
                SDL_GetWindowSize(win, &winW, &winH);
                bool down = (e.type == SDL_MOUSEBUTTONDOWN);
                int my = down ? e.button.y : e.motion.y;
                int mx = down ? e.button.x : e.motion.x;
                if (down && my > winH - BAR) dragging = true;
                if (e.type == SDL_MOUSEMOTION && !(e.motion.state & SDL_BUTTON_LMASK))
                    dragging = false;
                if (dragging) {
                    playing = false;
                    double f = (double)(mx - 16) / std::max(1, winW - 32);
                    f = f < 0 ? 0 : f > 1 ? 1 : f;
                    go((long)(f * maxExp));
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) dragging = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        double dt = std::min(0.1, (double)(now - last) / freq);
        last = now;
        if (playing) {
            acc += dt * SPEED[rate];
            long k = (long)acc; acc -= k;
            if (k) go(at + k);
            if (at >= maxExp) playing = false;
        }

        SDL_GetWindowSize(win, &winW, &winH);
        const int gridH = winH - BAR;
        SDL_SetRenderDrawColor(R, VOID.r, VOID.g, VOID.b, 255);
        SDL_RenderClear(R);

        const int pw = winW / cols, ph = gridH / rows;
        for (int i = 0; i < n; ++i) {
            SDL_Rect area{(i % cols) * pw, (i / cols) * ph, pw, ph};
            std::string cap = T[i].algo;
            long win2 = std::min(400L, std::max(60L, T[i].total / 8));
            drawPanel(R, T[i], S[i], area, win2, cap.c_str());
            SDL_SetRenderDrawColor(R, RULE.r, RULE.g, RULE.b, 255);
            SDL_RenderDrawRect(R, &area);
        }

        SDL_SetRenderDrawColor(R, CHROME.r, CHROME.g, CHROME.b, 255);
        SDL_Rect bar{0, winH - BAR, winW, BAR};
        SDL_RenderFillRect(R, &bar);
        SDL_SetRenderDrawColor(R, RULE.r, RULE.g, RULE.b, 255);
        SDL_RenderDrawLine(R, 0, winH - BAR, winW, winH - BAR);
        SDL_Rect track{16, winH - BAR + 10, winW - 32, 3};
        SDL_RenderFillRect(R, &track);
        double frac = maxExp ? (double)at / maxExp : 0;
        SDL_SetRenderDrawColor(R, LAMP.r, LAMP.g, LAMP.b, 255);
        SDL_Rect fill{16, winH - BAR + 10, (int)((winW - 32) * frac), 3};
        SDL_RenderFillRect(R, &fill);
        SDL_Rect knob{16 + (int)((winW - 32) * frac) - 4, winH - BAR + 6, 9, 11};
        SDL_RenderFillRect(R, &knob);

        std::string l = "EXPANSIONS " + commas(at) + " / " + commas(maxExp) +
                        "   SAME BUDGET FOR ALL FOUR";
        text(R, 16, winH - BAR + 24, l, CHALK, 2);
        const char *rl[5] = {"1X", "2X", "4X", "8X", "MAX"};
        std::string right = std::string(playing ? "PLAYING " : "PAUSED  ") + rl[rate];
        text(R, winW - 16 - (int)right.size() * 8, winH - BAR + 24, right, MUTED, 2);

        SDL_RenderPresent(R);
        if (!playing) SDL_Delay(12);
    }

    SDL_DestroyRenderer(R);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return true;
}

}  // namespace window
}  // namespace gs

#endif  // GS_HAVE_SDL
