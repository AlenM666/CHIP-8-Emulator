#!/bin/bash

# ── DAY-TO-DAY DEVELOPMENT (use this most of the time) ───────────────────────
#
#   Full symbols, no optimization, fast compile times.
#   Add -DENABLE_SANITIZERS=ON to catch memory bugs and undefined behaviour.
#
      # cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
      # cmake --build build/debug
      # ./build/debug/bin/CHIP8
#
#   With sanitizers:
cmake -B build/debug -G Ninja  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
./build/debug/bin/CHIP8
#
# ── SHIPPING / STEAM RELEASE BUILD ───────────────────────────────────────────
#
#   Maximum optimization, LTO enabled, dead code stripped.
#   This is what players run. Build this when you are ready to ship.
#
#       cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
#       cmake --build build/release
#       ./build/release/bin/CHIP8
#
# ── PROFILING (game feels slow, find out why) ────────────────────────────────
#
#   Optimized like Release but keeps symbols and frame pointers so that
#   tools like perf, Valgrind, gprof, or Tracy can tell you exactly
#   where the slowness is. Use this before optimizing anything.
#
#       cmake -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=Profile
#       cmake --build build/profile
#       ./build/profile/bin/CHIP8
#
# ── TIP: SEPARATE BUILD FOLDERS ──────────────────────────────────────────────
#
#   Keep a separate folder for each build type so you never have to
#   wait for a full reconfigure when switching between them:
#
#       cmake -B build/debug   -G Ninja -DCMAKE_BUILD_TYPE=Debug
#       cmake -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
#       cmake -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=Profile
#
#   Then just rebuild whichever you need:
#       cmake --build build/debug
#       cmake --build build/release
#       cmake --build build/profile



