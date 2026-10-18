#!/usr/bin/env bash
# Build build/oni-mobile.apk: the IL2CPP Android player from unity/player with
# its data replaced by the converted game data (convert_data.py --platform
# android --shaders gles). StreamingAssets is not in the APK: push it with
# tools/scripts/push_streaming_assets.sh.
#
# Usage: tools/scripts/make_apk.sh [converted data dir] [game data dir]
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
CONVERTED=$(cd "${1:-$ROOT/build/data-android}" && pwd)
GAME_DATA=${2:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data}
ANDROID=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data/PlaybackEngines/AndroidPlayer
BUILD_TOOLS=$(ls -d "$ANDROID"/SDK/build-tools/* | sort -V | tail -1)
JAVA_HOME=$ANDROID/OpenJDK
export JAVA_HOME PATH=$JAVA_HOME/bin:$PATH
WORK=$ROOT/build/apk-work
OUT=$ROOT/build/oni-mobile.apk

rm -rf "$WORK"
mkdir -p "$WORK/assets/bin/Data/Resources"
cp "$ROOT/unity/player/Builds/Android/oni.apk" "$WORK/base.apk"

# Drop the empty project's scenes and assets; keep the player's own engine files
# (unity default resources, Managed/Metadata, boot.config, unity_app_guid).
zip -q -d "$WORK/base.apk" \
    'assets/bin/Data/globalgamemanagers*' 'assets/bin/Data/level*' 'assets/bin/Data/sharedassets*' \
    'assets/bin/Data/resources.*' 'assets/bin/Data/Resources/unity_builtin_extra' \
    'assets/bin/Data/RuntimeInitializeOnLoads.json' 'assets/bin/Data/ScriptingAssemblies.json' >/dev/null

D=$WORK/assets/bin/Data
for f in globalgamemanagers globalgamemanagers.assets level0 level1 level2 \
         resources.assets resources.assets.resS sharedassets0.assets sharedassets0.assets.resS \
         sharedassets1.assets sharedassets1.assets.resS sharedassets2.assets sharedassets2.assets.resS; do
    [ -e "$CONVERTED/$f" ] && ln "$CONVERTED/$f" "$D/$f" 2>/dev/null || cp "$CONVERTED/$f" "$D/$f"
done
cp "$CONVERTED/Resources/unity_builtin_extra" "$D/Resources/"
# Unchanged game files: audio clips and the script tables.
for f in resources.resource RuntimeInitializeOnLoads.json ScriptingAssemblies.json; do
    cp "$GAME_DATA/$f" "$D/$f"
done

# Data goes in uncompressed (the player memory-maps and seeks in it).
(cd "$WORK" && zip -q -0 -r base.apk assets)
"$BUILD_TOOLS/zipalign" -f -P 16 4 "$WORK/base.apk" "$WORK/aligned.apk"

KEY=$ROOT/build/debug.keystore
if [ ! -e "$KEY" ]; then
    keytool -genkeypair -keystore "$KEY" -storepass android -keypass android -alias androiddebugkey \
        -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Android Debug,O=Android,C=US" >/dev/null
fi
"$BUILD_TOOLS/apksigner" sign --ks "$KEY" --ks-pass pass:android --out "$OUT" "$WORK/aligned.apk"
rm -rf "$WORK"
# Symbols of this APK's libil2cpp.so, for tools/scripts/profile_android.sh, kept
# per build id so an older APK on the phone still finds its own.
SYM=$ROOT/unity/player/Library/Bee/artifacts/Android/il2cppOutput/build/arm64-v8a/libil2cpp.sym.so
ID=$(file -L "$SYM" | grep -o 'BuildID\[sha1\]=[0-9a-f]*' | cut -d= -f2)
mkdir -p "$ROOT/build/android-symbols/$ID"
cp "$SYM" "$ROOT/build/android-symbols/$ID/"
ls -la "$OUT"
