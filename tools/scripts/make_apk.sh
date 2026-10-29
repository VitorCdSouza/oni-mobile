#!/usr/bin/env bash
# Build build/oni-mobile.apk: the IL2CPP Android player from unity/player with
# its data replaced by the converted game data (convert_data.py --platform
# android --shaders gles), and the game's StreamingAssets: the FMOD banks at
# assets/ (FMOD reads them from the APK) and everything else under
# assets/StreamingAssets/, which the game copies out on its first start
# (tools/runtime/StreamingAssets.cs). The converted bundles replace the original
# ones; the hires_* bundles are left out.
#
# The DLCs (Spaced Out, the planet packs, the cosmetics pack) are left out too,
# their bundles and banks, about 600 MB: the game turns a DLC on only when its
# bundle is there (DlcManager.CheckForDLCFileInstallation), and KFMOD.Initialize
# then drops its banks from the list. ONI_DLC=1 keeps them.
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
zip -q -d "$WORK/base.apk" 'assets/bin/Data/data.unity3d' \
    'assets/bin/Data/globalgamemanagers*' 'assets/bin/Data/level*' 'assets/bin/Data/sharedassets*' \
    'assets/bin/Data/resources.*' 'assets/bin/Data/Resources/unity_builtin_extra' \
    'assets/bin/Data/RuntimeInitializeOnLoads.json' 'assets/bin/Data/ScriptingAssemblies.json' >/dev/null

# The game's data goes into one LZ4HC archive, data.unity3d (tools/scripts/pack_data.py),
# which the player reads in place: ~1.6 GB of data in about 0.6 GB.
D=$WORK/assets/bin/Data
packed=()
for f in globalgamemanagers globalgamemanagers.assets level0 level1 level2 \
         resources.assets resources.assets.resS sharedassets0.assets sharedassets0.assets.resS \
         sharedassets1.assets sharedassets1.assets.resS sharedassets2.assets sharedassets2.assets.resS \
         Resources/unity_builtin_extra; do
    packed+=("$f=$CONVERTED/$f")
done
# Unchanged game files: audio clips, and the script tables, which stay beside the archive.
packed+=("resources.resource=$GAME_DATA/resources.resource")
"$ROOT/tools/venv/bin/python" -I "$ROOT/tools/scripts/pack_data.py" "$D/data.unity3d" "${packed[@]}"
for f in RuntimeInitializeOnLoads.json ScriptingAssemblies.json; do
    cp "$GAME_DATA/$f" "$D/$f"
done

# StreamingAssets as symlinks, which zip follows, so the ~1 GB is not copied twice.
S=$WORK/assets/StreamingAssets
mkdir -p "$S"
for f in "$GAME_DATA"/StreamingAssets/*; do
    name=$(basename "$f")
    case "$name" in
        hires_*) continue ;;
        expansion1_*|DLC[0-9]_*|dlc[0-9]_bundle|cosmetic[0-9]_bundle) [ "${ONI_DLC:-0}" = 1 ] || continue ;;&
        *.bank) ln -s "$f" "$WORK/assets/$name" ;;
        *) if [ -e "$CONVERTED/StreamingAssets/$name" ]; then ln -s "$CONVERTED/StreamingAssets/$name" "$S/$name"
           else ln -s "$f" "$S/$name"; fi ;;
    esac
done

# Data goes in uncompressed (the player seeks in data.unity3d, FMOD streams the
# banks, and StreamingAssets.cs copies the rest out byte for byte).
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
