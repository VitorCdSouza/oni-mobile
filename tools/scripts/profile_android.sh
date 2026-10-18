#!/usr/bin/env bash
# Sample the game running on the phone with simpleperf and write where the CPU
# time goes: per thread, per function in each thread, and per function with its
# callees (inclusive). Needs an APK built with BuildPlayer.ProfileableManifest
# (profileable from the shell) and the game running a save on the phone.
#
# Symbols: make_apk.sh keeps the libil2cpp symbols of the APK it packs in
# build/android-symbols; libunity's come from the Unity install, libSimDLL's
# from the sim build. They are matched to the samples by build id.
#
# Usage: tools/scripts/profile_android.sh [seconds] [output dir]
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DURATION=${1:-30}
OUT=${2:-$ROOT/build/profile/$(date +%Y%m%d-%H%M%S)}
ANDROID=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data/PlaybackEngines/AndroidPlayer
SIMPLEPERF=$ANDROID/NDK/simpleperf
HOST=$SIMPLEPERF/bin/linux/x86_64/simpleperf
PACKAGE=local.oni.mobile
DEVICE_DATA=/data/local/tmp/oni-perf.data

adb shell pidof "$PACKAGE" >/dev/null || { echo "$PACKAGE is not running on the phone" >&2; exit 1; }
mkdir -p "$OUT"
cd "$OUT"

# cpu-clock works without access to the hardware counters. Unity compiles
# libil2cpp with -fomit-frame-pointer, so call chains come from DWARF unwinding
# (.eh_frame), which copies the stack on each sample: keep the rate low.
adb shell simpleperf record --app "$PACKAGE" -e cpu-clock -f 400 --call-graph dwarf,16384 \
    --duration "$DURATION" -o "$DEVICE_DATA"
adb pull "$DEVICE_DATA" perf.data >/dev/null
adb shell rm -f "$DEVICE_DATA"

python3 "$SIMPLEPERF/binary_cache_builder.py" -i perf.data -lib \
    "$ROOT/build/android-symbols" \
    "$ANDROID/Variations/il2cpp/Release/Symbols/arm64-v8a" \
    "$ANDROID/Variations/il2cpp/Release_ThinLTO/Symbols/arm64-v8a" \
    "$ROOT/sim/build/android-arm64/standalone" >/dev/null

report() { "$HOST" report -i perf.data --symfs binary_cache "$@" 2>/dev/null; }
report --sort comm -n > threads.txt
report --sort comm,dso,symbol --percent-limit 0.2 > functions.txt
report --children --sort comm,symbol --percent-limit 1 > inclusive.txt
head -30 threads.txt
echo "Reports in $OUT"
