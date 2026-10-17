#!/usr/bin/env bash
# Build arm64 Android stubs for the native libraries the game imports but that do
# not exist on Android (Steam). Output: build/stubs/lib<name>.so
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
GAME_MANAGED=${1:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data/Managed}
EDITOR=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data
MB=$EDITOR/MonoBleedingEdge
CC=$EDITOR/PlaybackEngines/AndroidPlayer/NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang
OUT=$ROOT/build/stubs

mkdir -p "$OUT"
"$MB/bin/mcs" -sdk:4.5 -out:"$ROOT/build/picktypes/ListImports.exe" -r:"$ROOT/build/picktypes/Mono.Cecil.dll" \
    -r:"$MB/lib/mono/4.5/Facades/netstandard.dll" -r:"$MB/lib/mono/4.5/Facades/System.Runtime.dll" \
    "$ROOT/tools/picktypes/ListImports.cs" "$ROOT/tools/picktypes/NameResolver.cs"
for lib in steam_api sdkencryptedappticket; do
    for dll in "$GAME_MANAGED"/*.dll; do
        "$MB/bin/mono" "$ROOT/build/picktypes/ListImports.exe" "$dll" "$GAME_MANAGED" "$lib" 2>/dev/null || true
    done | sort -u > "$OUT/$lib.txt"
    python3 -I "$ROOT/tools/stubs/gen_stub.py" "$OUT/$lib.txt" "$OUT/$lib.c"
    "$CC" -O2 -shared -fPIC -Wl,--no-undefined -o "$OUT/lib$lib.so" "$OUT/$lib.c"
    echo "lib$lib.so: $(wc -l < "$OUT/$lib.txt") functions"
done
