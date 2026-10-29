// viz.h -- writes a self-contained replay page and opens it in a browser.
//
// There is no server and no port. The trace is inlined into a copy of
// web/player.html, so the page works straight off the filesystem on any
// machine with a browser.
//
// Provided. You should not need to change this file.

#ifndef GS_VIZ_H
#define GS_VIZ_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef GS_ROOT
#define GS_ROOT "."
#endif

namespace gs {
namespace viz {

inline std::string slurp(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline void openUrl(const std::string &url) {
#if defined(_WIN32)
    (void)!std::system(("start \"\" \"" + url + "\"").c_str());
#elif defined(__APPLE__)
    (void)!std::system(("open '" + url + "' >/dev/null 2>&1").c_str());
#else
    (void)!std::system(("xdg-open '" + url + "' >/dev/null 2>&1 &").c_str());
#endif
}

inline void launchTemplate(const std::string &page, const std::string &traceText,
                           const std::string &tag) {
    const std::string root = GS_ROOT;
    const std::string tmpl = slurp(root + "/web/" + page);
    if (tmpl.empty()) {
        std::cerr << "viz: cannot read " << root << "/web/" << page << "\n";
        return;
    }
    const std::string marker = "__TRACE_PAYLOAD__";
    const size_t at = tmpl.find(marker);
    if (at == std::string::npos) {
        std::cerr << "viz: " << page << " has no payload marker\n";
        return;
    }
    std::string html = tmpl.substr(0, at) + traceText +
                       tmpl.substr(at + marker.size());

    const std::string dir = root + "/runs";
#ifdef _WIN32
    (void)!std::system(("if not exist \"" + dir + "\" mkdir \"" + dir + "\"").c_str());
#else
    (void)!std::system(("mkdir -p '" + dir + "'").c_str());
#endif
    const std::string out = dir + "/" + tag + ".html";
    { std::ofstream f(out, std::ios::binary); f << html; }

    std::cerr << "viz: " << out << "\n";
#if defined(_WIN32)
    (void)!std::system(("start \"\" \"" + out + "\"").c_str());
#elif defined(__APPLE__)
    (void)!std::system(("open '" + out + "' >/dev/null 2>&1").c_str());
#else
    (void)!std::system(("xdg-open '" + out + "' >/dev/null 2>&1 &").c_str());
#endif
}

inline void launch(const std::string &traceText, const std::string &tag) {
    launchTemplate("player.html", traceText, tag);
}

}  // namespace viz
}  // namespace gs

#endif
