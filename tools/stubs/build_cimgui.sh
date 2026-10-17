#!/usr/bin/env bash
# Build cimgui 1.82 with docking (the version the game ships) for arm64 Android.
# Source: vendor/cimgui-src/cimgui (git clone --branch 1.82dock --recurse-submodules
# https://github.com/cimgui/cimgui). Output: build/stubs/libcimgui.so
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
SRC=$ROOT/vendor/cimgui-src/cimgui
CXX=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data/PlaybackEngines/AndroidPlayer/NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang++
mkdir -p "$ROOT/build/stubs"
# Same flags as cimgui's own Makefile; libc++ linked statically so nothing else ships.
"$CXX" -O2 -fno-exceptions -fno-rtti -shared -fPIC -static-libstdc++ -I"$SRC/imgui" \
    -o "$ROOT/build/stubs/libcimgui.so" \
    "$SRC/cimgui.cpp" "$SRC"/imgui/imgui.cpp "$SRC"/imgui/imgui_draw.cpp "$SRC"/imgui/imgui_demo.cpp \
    "$SRC"/imgui/imgui_tables.cpp "$SRC"/imgui/imgui_widgets.cpp
