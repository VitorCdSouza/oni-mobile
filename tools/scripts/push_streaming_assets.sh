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
REMOTE=/sdcard/Android/data/local.oni.mobile/files/StreamingAssets

# adb push copies symlinks as symlinks, which the phone's storage rejects, so
# push each original entry directly and the converted bundles on top. adbd may
# not create directories there (secure_mkdirs fails), but the shell can.
# Some directory names have spaces, so send the shell one quoted mkdir per line.
(cd "$GAME_DATA/StreamingAssets" && find . -type d ! -path './hires_*' -printf "mkdir -p '$REMOTE/%P'\n") | adb shell sh
for f in "$GAME_DATA"/StreamingAssets/*; do
    name=$(basename "$f")
    case "$name" in hires_*) continue ;; esac
    [ -e "$CONVERTED/StreamingAssets/$name" ] && continue
    adb push --sync "$f" "$REMOTE/"
done
adb push --sync "$CONVERTED"/StreamingAssets/* "$REMOTE/"
