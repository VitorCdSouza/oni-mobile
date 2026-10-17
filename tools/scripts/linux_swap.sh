#!/usr/bin/env bash
# Assemble build/linux-swap: the IL2CPP Linux player from unity/player with the
# original game data linked in (no copies), to test the data swap on the PC.
# With an overlay dir (output of convert_data.py), its files replace the originals.
#
# Usage: tools/scripts/linux_swap.sh [overlay dir] [game data dir]
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OVERLAY=${1:+$(cd "$1" && pwd)}
GAME_DATA=${2:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data}
BUILD=$ROOT/unity/player/Builds/Linux
OUT=$ROOT/build/linux-swap

# Links every file of $1 into $2, taking the overlay's copy when there is one.
link_tree() {
    local src=$1 dst=$2 rel=$3
    mkdir -p "$dst"
    for f in "$src"/*; do
        local name; name=$(basename "$f")
        if [ -d "$f" ]; then
            link_tree "$f" "$dst/$name" "$rel/$name"
        elif [ -n "$OVERLAY" ] && [ -e "$OVERLAY$rel/$name" ]; then
            ln -s "$OVERLAY$rel/$name" "$dst/$name"
        else
            ln -s "$f" "$dst/$name"
        fi
    done
    # Overlay-only files, such as a .resS the original did not have.
    if [ -n "$OVERLAY" ] && [ -d "$OVERLAY$rel" ]; then
        for f in "$OVERLAY$rel"/*; do
            [ -f "$f" ] && [ ! -e "$dst/$(basename "$f")" ] && ln -s "$f" "$dst/"
        done
    fi
    return 0
}

rm -rf "$OUT"
mkdir -p "$OUT/OxygenNotIncluded_Data"
cp "$BUILD"/OxygenNotIncluded "$BUILD"/UnityPlayer.so "$BUILD"/GameAssembly.so "$BUILD"/libdecor* "$OUT/"
cp -r "$BUILD/OxygenNotIncluded_Data/il2cpp_data" "$OUT/OxygenNotIncluded_Data/"
for f in "$GAME_DATA"/*; do
    name=$(basename "$f")
    case $name in Managed|MonoBleedingEdge) continue ;; esac
    if [ -d "$f" ]; then
        link_tree "$f" "$OUT/OxygenNotIncluded_Data/$name" "/$name"
    elif [ -n "$OVERLAY" ] && [ -e "$OVERLAY/$name" ]; then
        ln -s "$OVERLAY/$name" "$OUT/OxygenNotIncluded_Data/"
    else
        ln -s "$f" "$OUT/OxygenNotIncluded_Data/"
    fi
done
if [ -n "$OVERLAY" ]; then
    for f in "$OVERLAY"/*; do
        [ -f "$f" ] && [ ! -e "$OUT/OxygenNotIncluded_Data/$(basename "$f")" ] && ln -s "$f" "$OUT/OxygenNotIncluded_Data/"
    done
fi
# Keeps Steamworks from relaunching the game through Steam.
echo 457140 > "$OUT/steam_appid.txt"
