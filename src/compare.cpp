// compare.cpp -- stitches several traces of the same map into one page.
//
// Not part of any stage. The run script builds it, runs each algorithm with
// --trace, and hands the trace files here.
//
//   bin/compare runs/x.dfs.trace runs/x.bfs.trace ... -o runs/compare-x.html
//
// Provided.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <vector>

#include "viz.h"
#include "window.h"

int main(int argc, char **argv) {
    std::vector<std::string> inputs;
    std::string out = "runs/compare.html", title = "gridsearch - side by side";
    bool wantWin = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-o" && i + 1 < argc) out = argv[++i];
        else if (a == "-t" && i + 1 < argc) title = argv[++i];
        else if (a == "--window" || a == "--win") wantWin = true;
        else inputs.push_back(a);
    }
    if (inputs.empty()) {
        std::cerr << "compare: no trace files given\n";
        return 2;
    }

    std::vector<std::string> texts;
    std::string payload;
    for (const std::string &p : inputs) {
        std::string t = gs::viz::slurp(p);
        if (t.empty()) { std::cerr << "compare: cannot read " << p << "\n"; return 2; }
        texts.push_back(t);
        payload += "===RUN===\n";
        payload += t;
        if (t.back() != '\n') payload += '\n';
    }

    if (wantWin) {
        if (!gs::window::available)
            std::cerr << "compare: this build has no SDL2, writing the browser page instead\n";
        else if (gs::window::replayGrid(texts, title))
            return 0;
        else
            std::cerr << "compare: could not open a window, writing the browser page instead\n";
    }

    const std::string tmpl = gs::viz::slurp(std::string(GS_ROOT) + "/web/compare.html");
    if (tmpl.empty()) { std::cerr << "compare: missing web/compare.html\n"; return 2; }
    const std::string mark = "__RUNS_PAYLOAD__";
    size_t at = tmpl.find(mark);
    if (at == std::string::npos) { std::cerr << "compare: template has no marker\n"; return 2; }

    { std::ofstream f(out, std::ios::binary);
      f << tmpl.substr(0, at) << payload << tmpl.substr(at + mark.size()); }
    std::cerr << "compare: " << out << "\n";
    gs::viz::openUrl(out);
    return 0;
}
