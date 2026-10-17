#!/usr/bin/env bash
# Copy the arm64 native plugins (and FMOD's jar) into the player project as
# Android-only plugins: unity/player/Assets/Plugins/Android.
#
# Needs: sim/build/android-arm64/standalone/libSimDLL.so (make -C sim/port standalone with the
# NDK), build/stubs (tools/stubs/build.sh, tools/stubs/build_cimgui.sh) and vendor/fmod
# (FMOD Engine 2.03.12 for Android).
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DEST=$ROOT/unity/player/Assets/Plugins/Android
FMOD=$ROOT/vendor/fmod/fmodstudioapi20312android
STRIP=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data/PlaybackEngines/AndroidPlayer/NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip

meta() {  # meta <file> <android settings: " {}" or a newline and indented keys>
    local guid; guid=$(printf '%s' "oni-mobile/android/$(basename "$1")" | md5sum | cut -c1-32)
    cat > "$1.meta" <<EOF
fileFormatVersion: 2
guid: $guid
PluginImporter:
  externalObjects: {}
  serializedVersion: 2
  iconMap: {}
  executionOrder: {}
  defineConstraints: []
  isPreloaded: 0
  isOverridable: 0
  isExplicitlyReferenced: 0
  validateReferences: 1
  platformData:
  - first:
      : Any
    second:
      enabled: 0
      settings:
        Exclude Android: 0
        Exclude Editor: 1
        Exclude Linux64: 1
  - first:
      Android: Android
    second:
      enabled: 1
      settings:$2
  - first:
      Any:
    second:
      enabled: 0
      settings: {}
  - first:
      Editor: Editor
    second:
      enabled: 0
      settings:
        DefaultValueInitialized: true
  - first:
      Standalone: Linux64
    second:
      enabled: 0
      settings:
        CPU: None
  userData:
  assetBundleName:
  assetBundleVariant:
EOF
}

rm -rf "$DEST"
mkdir -p "$DEST"
for so in "$ROOT/sim/build/android-arm64/standalone/libSimDLL.so" \
          "$ROOT/build/stubs/libsteam_api.so" "$ROOT/build/stubs/libsdkencryptedappticket.so" \
          "$ROOT/build/stubs/libcimgui.so" \
          "$FMOD/api/core/lib/arm64-v8a/libfmod.so" "$FMOD/api/studio/lib/arm64-v8a/libfmodstudio.so" \
          "$FMOD/plugins/resonance_audio/lib/arm64-v8a/libresonanceaudio.so"; do
    "$STRIP" --strip-debug -o "$DEST/$(basename "$so")" "$so"
    meta "$DEST/$(basename "$so")" "
        CPU: ARM64"
done
cp "$FMOD/api/core/lib/fmod.jar" "$DEST/"
meta "$DEST/fmod.jar" " {}"
ls -la "$DEST"

# App icon: the PC game's icon, as Steam installs it for the desktop entry.
ICON=$HOME/.local/share/icons/hicolor/256x256/apps/steam_icon_457140.png
mkdir -p "$ROOT/unity/player/Assets/Icon"
cp "$ICON" "$ROOT/unity/player/Assets/Icon/icon.png"
