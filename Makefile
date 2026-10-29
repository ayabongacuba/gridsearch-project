# gridsearch -- one binary per stage.
#
#   make            build every stage
#   make dfs        build just that stage
#   make STUBS=1    force building from src/ even if solutions/ is present
#   make compare    build the side-by-side comparison tool
#   make clean
#
# Each stage builds from solutions/<stage>.cpp when that file exists, and
# from src/<stage>.cpp otherwise. Delete the solutions/ directory before
# handing this to students and everything falls back to src/ on its own.

CXX      ?= g++
CXXSTD   ?= -std=c++17
OPT      ?= -O2
ROOT     := $(CURDIR)
CXXFLAGS := $(CXXSTD) $(OPT) -Wall -Iinclude -DGS_ROOT='"$(ROOT)"'
HDRS     := include/grid.h include/search.h include/trace.h include/viz.h \
            include/live.h include/matrix.h include/window.h

# stage -> source file: solutions/ when present, src/ otherwise
ifdef STUBS
  pick = src/$(1).cpp
else
  pick = $(if $(wildcard solutions/$(1).cpp),solutions/$(1).cpp,src/$(1).cpp)
endif

# every binary needs Stage 0 and the provided helpers
BASE      = $(call pick,util) src/provided.cpp

# A native replay window is used when SDL2 is installed, and quietly skipped
# when it is not -- everything still builds, --window just falls back to the
# browser.  Linux: apt install libsdl2-dev   macOS: brew install sdl2
SDL_OK   := $(shell pkg-config --exists sdl2 && echo yes)
ifeq ($(SDL_OK),yes)
  CXXFLAGS += -DGS_HAVE_SDL $(shell pkg-config --cflags sdl2)
  LDLIBS   += $(shell pkg-config --libs sdl2)
endif
COMMON   := src/window_sdl.cpp

SEARCH := dfs bfs dijkstra astar
STAGES := $(SEARCH) floyd tsp
BINS   := $(addprefix bin/,$(STAGES)) bin/compare

.PHONY: all clean compare $(STAGES)
all: $(BINS)

$(STAGES): %: bin/%
compare: bin/compare

define STAGE_RULE
bin/$(1): $$(call pick,$(1)) src/main_search.cpp $$(COMMON) $$(BASE) $(HDRS)
	@mkdir -p bin
	@echo "  build $(1)  <- $$(call pick,$(1))"
	@$$(CXX) $$(CXXFLAGS) -o $$@ $$(call pick,$(1)) src/main_search.cpp $$(BASE) $$(COMMON) -pthread $$(LDLIBS)
endef
$(foreach s,$(SEARCH),$(eval $(call STAGE_RULE,$(s))))

# Stages 5 and 6 reuse your Dijkstra: the tour is built out of real paths.
bin/floyd: $(call pick,floyd) $(call pick,dijkstra) src/main_matrix.cpp $(COMMON) $(BASE) $(HDRS)
	@mkdir -p bin
	@echo "  build floyd  <- $(call pick,floyd) + $(call pick,dijkstra)"
	@$(CXX) $(CXXFLAGS) -o $@ $(call pick,floyd) $(call pick,dijkstra) src/main_matrix.cpp $(BASE) $(COMMON) -pthread $(LDLIBS)

bin/tsp: $(call pick,tsp) $(call pick,floyd) $(call pick,dijkstra) src/main_tour.cpp $(COMMON) $(BASE) $(HDRS)
	@mkdir -p bin
	@echo "  build tsp  <- $(call pick,tsp) + $(call pick,floyd) + $(call pick,dijkstra)"
	@$(CXX) $(CXXFLAGS) -o $@ $(call pick,tsp) $(call pick,floyd) $(call pick,dijkstra) src/main_tour.cpp $(BASE) $(COMMON) -pthread $(LDLIBS)

bin/compare: src/compare.cpp $(COMMON) include/viz.h include/window.h
	@mkdir -p bin
	@$(CXX) $(CXXFLAGS) -o $@ src/compare.cpp $(BASE) $(COMMON) $(LDLIBS)

# ---------------------------------------------------------------- tests
# Catch2 v2 single header, vendored in tests/. Nothing to install.
#   make test          build and run every stage's tests
#   make test-dfs      just this stage
TESTS    := dfs bfs dijkstra astar floyd tsp
TESTFLAGS := $(CXXSTD) $(OPT) -Iinclude -Itests -DGS_ROOT='"$(ROOT)"' \
             -DGS_MAPS='"$(ROOT)/maps"' -DGS_EXPECTED='"$(ROOT)/expected"'

.PHONY: test test-util $(addprefix test-,$(TESTS))
test: test-util $(addprefix test-,$(TESTS))

# Catch2's main, built once. Every test binary links this instead of
# recompiling the whole framework.
bin/catch_main.o: tests/catch_main.cpp tests/catch.hpp
	@mkdir -p bin
	@echo "  build catch_main.o (once)"
	@$(CXX) $(CXXSTD) $(OPT) -Itests -c -o $@ tests/catch_main.cpp

# Stage 0 exercises the provided headers only, so it links nothing.
bin/test_util: tests/test_util.cpp tests/harness.h $(BASE) $(HDRS) bin/catch_main.o
	@mkdir -p bin
	@echo "  build test_util"
	@$(CXX) $(TESTFLAGS) -o $@ tests/test_util.cpp $(BASE) bin/catch_main.o

test-util: bin/test_util
	@./bin/test_util

SEARCHTESTS := dfs bfs dijkstra astar

define TEST_RULE
bin/test_$(1): tests/test_$(1).cpp tests/harness.h $$(call pick,$(1)) $$(BASE) $(HDRS) bin/catch_main.o
	@mkdir -p bin
	@echo "  build test_$(1)  <- $$(call pick,$(1))"
	@$$(CXX) $$(TESTFLAGS) -o $$@ tests/test_$(1).cpp $$(call pick,$(1)) $$(BASE) bin/catch_main.o

test-$(1): bin/test_$(1)
	@./bin/test_$(1)
endef
$(foreach t,$(SEARCHTESTS),$(eval $(call TEST_RULE,$(t))))

# Stage 5 needs your Dijkstra; stage 6 needs both it and your matrix.
bin/test_floyd: tests/test_floyd.cpp tests/harness.h $(call pick,floyd) $(call pick,dijkstra) $(BASE) $(HDRS) bin/catch_main.o
	@mkdir -p bin
	@echo "  build test_floyd  <- $(call pick,floyd) + $(call pick,dijkstra)"
	@$(CXX) $(TESTFLAGS) -o $@ tests/test_floyd.cpp $(call pick,floyd) $(call pick,dijkstra) $(BASE) bin/catch_main.o

test-floyd: bin/test_floyd
	@./bin/test_floyd

bin/test_tsp: tests/test_tsp.cpp tests/harness.h $(call pick,tsp) $(call pick,floyd) $(call pick,dijkstra) $(BASE) $(HDRS) bin/catch_main.o
	@mkdir -p bin
	@echo "  build test_tsp  <- $(call pick,tsp) + $(call pick,floyd) + $(call pick,dijkstra)"
	@$(CXX) $(TESTFLAGS) -o $@ tests/test_tsp.cpp $(call pick,tsp) $(call pick,floyd) $(call pick,dijkstra) $(BASE) bin/catch_main.o

test-tsp: bin/test_tsp
	@./bin/test_tsp

clean:
	rm -rf bin runs
