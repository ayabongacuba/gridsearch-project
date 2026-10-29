// live.h -- a very small localhost HTTP server, used only to stream a search
// to the browser while it is still running.
//
// It binds to 127.0.0.1 on an ephemeral port, serves the player page, and
// hands out the trace as a growing byte stream. There is nothing here worth
// reading unless you are curious: it is transport, not algorithms.
//
// Provided. You should not need to change this file.

#ifndef GS_LIVE_H
#define GS_LIVE_H

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socket_t = SOCKET;
  #define GS_CLOSE closesocket
  #define GS_BAD INVALID_SOCKET
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  using socket_t = int;
  #define GS_CLOSE ::close
  #define GS_BAD (-1)
#endif

namespace gs {
namespace live {

struct Server {
    std::mutex mu;
    std::string body;            // the trace so far
    std::string page;            // player.html
    std::atomic<bool> done{false};
    std::atomic<bool> quit{false};
    std::atomic<bool> seen{false};   // has a browser connected at all
    socket_t fd = GS_BAD;
    int port = 0;
    std::thread th;

    void append(const std::string &s) {
        std::lock_guard<std::mutex> g(mu);
        body += s;
    }
};

inline Server &server() { static Server s; return s; }

inline void sendAll(socket_t c, const char *p, size_t n) {
    while (n) {
        int k = (int)send(c, p, (int)n, 0);
        if (k <= 0) return;
        p += k; n -= k;
    }
}

inline void respond(socket_t c, const std::string &type, const std::string &b) {
    char h[256];
    int n = std::snprintf(h, sizeof h,
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n",
        type.c_str(), b.size());
    sendAll(c, h, (size_t)n);
    sendAll(c, b.data(), b.size());
}

inline void serve() {
    Server &S = server();
    while (!S.quit.load()) {
        socket_t c = accept(S.fd, nullptr, nullptr);
        if (c == GS_BAD) { if (S.quit.load()) break; continue; }
        char req[2048] = {0};
        int n = (int)recv(c, req, sizeof(req) - 1, 0);
        if (n <= 0) { GS_CLOSE(c); continue; }
        S.seen.store(true);

        std::string line(req, req + n);
        size_t sp = line.find(' ');
        size_t sp2 = line.find(' ', sp + 1);
        std::string path = (sp == std::string::npos || sp2 == std::string::npos)
                               ? "/" : line.substr(sp + 1, sp2 - sp - 1);

        if (path.rfind("/events", 0) == 0) {
            size_t from = 0;
            if (size_t q = path.find("from="); q != std::string::npos)
                from = (size_t)strtoull(path.c_str() + q + 5, nullptr, 10);
            std::string chunk;
            bool fin;
            {
                std::lock_guard<std::mutex> g(S.mu);
                if (from < S.body.size()) chunk = S.body.substr(from);
                fin = S.done.load();
            }
            respond(c, "text/plain", (fin ? "" : "") + chunk);
            // a trailing marker line tells the client the stream is complete
            if (fin) {
                // nothing more to do; the client checks /status
            }
        } else if (path.rfind("/status", 0) == 0) {
            respond(c, "text/plain", S.done.load() ? "done" : "running");
        } else if (path.rfind("/quit", 0) == 0) {
            respond(c, "text/plain", "bye");
            S.quit.store(true);
        } else {
            respond(c, "text/html; charset=utf-8", S.page);
        }
        GS_CLOSE(c);
    }
}

// Returns the port, or 0 on failure.
inline int start(const std::string &playerHtml) {
    Server &S = server();
#if defined(_WIN32)
    WSADATA w; if (WSAStartup(MAKEWORD(2, 2), &w) != 0) return 0;
#endif
    S.page = playerHtml;
    S.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (S.fd == GS_BAD) return 0;
    int yes = 1;
    setsockopt(S.fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof yes);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = 0;                                  // let the OS pick
    if (bind(S.fd, (sockaddr *)&a, sizeof a) != 0) { GS_CLOSE(S.fd); return 0; }
    socklen_t len = sizeof a;
    getsockname(S.fd, (sockaddr *)&a, &len);
    S.port = ntohs(a.sin_port);
    if (listen(S.fd, 8) != 0) { GS_CLOSE(S.fd); return 0; }
    S.th = std::thread(serve);
    return S.port;
}

inline void finish() { server().done.store(true); }

// Keep serving until the browser says it is finished, or the timeout expires.
inline void waitForClient(int seconds) {
    Server &S = server();
    auto t0 = std::chrono::steady_clock::now();
    while (!S.quit.load()) {
        auto dt = std::chrono::steady_clock::now() - t0;
        if (std::chrono::duration_cast<std::chrono::seconds>(dt).count() >= seconds)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
    S.quit.store(true);
    GS_CLOSE(S.fd);
    if (S.th.joinable()) S.th.detach();
}

}  // namespace live
}  // namespace gs

#endif
