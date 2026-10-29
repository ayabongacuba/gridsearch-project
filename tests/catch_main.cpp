// The Catch2 entry point, compiled once and linked into every test binary.
// Building this is the slow part of any test build, so it is cached.
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
