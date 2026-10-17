#!/usr/bin/env bash
# Push StreamingAssets to the phone, where the patched game reads it:
# /sdcard/Android/data/local.oni.mobile/files/StreamingAssets.
# Converted bundles (convert_data.py output) replace the original ones; the
# hires_* bundles are left out.
#
# Usage: tools/scripts/push_streaming_assets.sh [converted data dir] [game data dir]
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
CONVERTED=$(cd "${1:-$ROOT/build/data-android}" && pwd)
GAME_DATA=${2:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data}
STAGE=$ROOT/build/streaming-stage
REMOTE=/sdcard/Android/data/local.oni.mobile/files/StreamingAssets

# Stage with hard links/symlinks so adb push sends one tree.
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -rs "$GAME_DATA/StreamingAssets/." "$STAGE/"
rm -f "$STAGE"/hires_*
for f in "$CONVERTED"/StreamingAssets/*; do
    ln -sf "$f" "$STAGE/$(basename "$f")"
done
adb shell mkdir -p "$REMOTE"
adb push --sync "$STAGE/." "$REMOTE/"
