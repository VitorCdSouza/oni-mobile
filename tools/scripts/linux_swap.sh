#!/usr/bin/env bash
# Assemble build/linux-swap: the IL2CPP Linux player from unity/player with the
# original game data linked in (no copies), to test the data swap on the PC.
set -euo pipefail

GAME_DATA=${1:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$ROOT/unity/player/Builds/Linux
OUT=$ROOT/build/linux-swap

rm -rf "$OUT"
mkdir -p "$OUT/OxygenNotIncluded_Data"
cp "$BUILD"/OxygenNotIncluded "$BUILD"/UnityPlayer.so "$BUILD"/GameAssembly.so "$BUILD"/libdecor* "$OUT/"
cp -r "$BUILD/OxygenNotIncluded_Data/il2cpp_data" "$OUT/OxygenNotIncluded_Data/"
for f in "$GAME_DATA"/*; do
    case $(basename "$f") in Managed|MonoBleedingEdge) continue ;; esac
    ln -s "$f" "$OUT/OxygenNotIncluded_Data/"
done
# Keeps Steamworks from relaunching the game through Steam.
echo 457140 > "$OUT/steam_appid.txt"
