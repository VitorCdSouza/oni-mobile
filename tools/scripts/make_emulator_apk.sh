#!/usr/bin/env bash
# An APK for the Android emulator (x86_64 image running arm64 code through its NDK
# translation), made from build/oni-mobile.apk: the native libraries get their SEVL/WFE
# hints replaced by NOP (tools/scripts/emu_patch_hints.py), which the translation does not
# implement (SIGILL in libunity.so's spinlocks). Same key, so it installs over the phone build.
#
# Usage: tools/scripts/make_emulator_apk.sh [in.apk] [out.apk]
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
IN=$(realpath "${1:-$ROOT/build/oni-mobile.apk}")
OUT=$(realpath -m "${2:-$ROOT/build/oni-mobile-emu.apk}")
BUILD_TOOLS=$(ls -d "$HOME"/Unity/Hub/Editor/6000.3.5f2/Editor/Data/PlaybackEngines/AndroidPlayer/SDK/build-tools/* | tail -1)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

cp "$IN" "$WORK/base.apk"
(cd "$WORK" && unzip -q base.apk 'lib/arm64-v8a/*.so')
python3 -I "$ROOT/tools/scripts/emu_patch_hints.py" "$WORK"/lib/arm64-v8a/*.so
(cd "$WORK" && zip -q base.apk lib/arm64-v8a/*.so)
"$BUILD_TOOLS/zipalign" -f -P 16 4 "$WORK/base.apk" "$WORK/aligned.apk"
"$BUILD_TOOLS/apksigner" sign --ks "$ROOT/build/debug.keystore" --ks-pass pass:android --out "$OUT" "$WORK/aligned.apk"
ls -la "$OUT"
