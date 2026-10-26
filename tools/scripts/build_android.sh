#!/usr/bin/env bash
# Run the Unity Android build of unity/player (BuildPlayer.Android) inside a
# memory-capped scope. The IL2CPP steps take most of the machine's 15 GB; with a
# browser, Steam and the like open, a fixed cap still pushed the whole desktop
# into swap and froze it. The cap here is what is available now minus a margin,
# and the build gets no swap: if it runs out, the kernel kills Unity, not the
# desktop.
#
# Usage: tools/scripts/build_android.sh [linux]   (environment passes through, e.g.
#        ONI_IL2CPP_ARGS="--compiler-flags=-O2")
# With "linux" it builds the IL2CPP Linux player (BuildPlayer.Linux, Builds/Linux) for the
# PC checks of tools/scripts/linux_swap.sh and ui_check.sh; the cap protects the desktop
# the same way, and a build that hits it is killed.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
MARGIN_MB=1536
MIN_MB=9216
TARGET=Android
if [ "${1:-}" = linux ]; then
    TARGET=Linux
    MIN_MB=6144
fi

available=$(awk '/^MemAvailable:/ {print int($2 / 1024)}' /proc/meminfo)
cap=$((available - MARGIN_MB))
if [ "$cap" -lt "$MIN_MB" ]; then
    echo "only ${available} MB available; the build needs about $((MIN_MB + MARGIN_MB)) MB." >&2
    echo "Close other programs (browser, Steam, games) and run again." >&2
    exit 1
fi
echo "building with a ${cap} MB cap (${available} MB available)"

cd "$ROOT/unity/player"
status=0
systemd-run --user --scope -q -p MemoryMax="${cap}M" -p MemoryHigh="$((cap - 512))M" -p MemorySwapMax=0 \
    "$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Unity" -batchmode -nographics -quit -projectPath . \
    -buildTarget "$([ $TARGET = Linux ] && echo Linux64 || echo Android)" -executeMethod "BuildPlayer.$TARGET" \
    -logFile "Logs/build-${TARGET,,}.log" || status=$?
grep -E "\[BuildPlayer\]|error CS" "Logs/build-${TARGET,,}.log" | tail -5 || true
# The build writes the PlayerSettings it set (compiler arguments and such) into
# the tracked project settings; they are set again on every build.
git -C "$ROOT" checkout -q unity/player/ProjectSettings/ProjectSettings.asset
exit "$status"
