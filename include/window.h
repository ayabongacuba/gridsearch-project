// window.h -- replay a search in a real window instead of a browser.
//
// Built only when SDL2 is present; the Makefile detects it with pkg-config
// and defines GS_HAVE_SDL. Without it, --window falls back to the browser
// and says so, so the project still builds on a bare machine.
//
// Controls
//   space          play / pause
//   left / right   step one event
//   , / .          jump 100 events
//   1 2 3 4 5      speed: 1x 2x 4x 8x max
//   r              restart
//   f              fit the window to the map
//   q or escape    quit
//   click / drag   scrub along the bar at the bottom
//
// Provided. You should not need to change this file.

#ifndef GS_WINDOW_H
#define GS_WINDOW_H

#include <string>
#include <vector>

namespace gs {
namespace window {

#ifdef GS_HAVE_SDL
inline constexpr bool available = true;
#else
inline constexpr bool available = false;
#endif

// Returns false if the window could not be opened (no SDL, no display).
bool replay(const std::string &traceText, const std::string &title);

// Several traces of the same map, tiled and stepped together. The panels are
// synchronised on EXPANSIONS, not on wall clock: at any point every panel has
// expanded the same number of cells, so what you are comparing is what each
// algorithm did with the same budget.
bool replayGrid(const std::vector<std::string> &traces, const std::string &title);

}  // namespace window
}  // namespace gs

#endif
