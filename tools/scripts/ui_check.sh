#!/usr/bin/env bash
# Check the phone HUD (tools/layout) on the PC, in a 1600x720 window with
# ONI_MOBILE_UI=1, loading a copy of a save from a scratch config directory, so
# the real saves and settings are never touched. The probe (tools/layout/Probe.cs)
# runs the steps and leaves its screenshots and UI dumps in the output directory.
#
# By default it runs the IL2CPP Linux player of build/linux-swap
# (tools/scripts/linux_swap.sh). With --mono it runs the PC's own Mono build of the
# game instead, with the HUD compiled here in seconds: the quick loop for layout work.
# Both need tools/scripts/setup_player_project.sh to have run.
#
# Usage: tools/scripts/ui_check.sh [--mono] <save.sav> <out dir> [steps]
#   steps: e.g. "state:build;shot:build;dump:build" (default: shot:idle;dump:idle),
#   or "play" to leave the game open to try by hand, without the probe.
set -euo pipefail

MONO=0
if [ "${1:-}" = --mono ]; then
    MONO=1
    shift
fi
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
SAVE=$1
OUT=$(mkdir -p "$2" && cd "$2" && pwd)
STEPS=${3:-shot:idle;dump:idle}
GAME=$HOME/.steam/steam/steamapps/common/OxygenNotIncluded
REAL="$HOME/.config/unity3d/Klei/Oxygen Not Included"
CONFIG=$OUT/xdg/unity3d/Klei/Oxygen\ Not\ Included

mkdir -p "$CONFIG/save_files"
cp "$SAVE" "$CONFIG/save_files/check.sav"
# The real settings, minus the save and the screen, with the phone's UI scale (175%) and
# without Spaced Out (EXPANSION1_ID), which the game turns on when the setting is missing.
grep -vE '^\s*(SaveFilenameKey/|AutoResumeSaveFile|ResolutionWidth|ResolutionHeight|FullScreen|UIScalePref|EXPANSION1_ID\.ENABLED):' \
    "$REAL/kplayerprefs.yaml" > "$CONFIG/kplayerprefs.yaml"
sed -i -e "s|^strings:|strings:\n  AutoResumeSaveFile: $CONFIG/save_files/check.sav|" \
    -e 's|^ints:|ints:\n  ResolutionWidth: 1600\n  ResolutionHeight: 720\n  FullScreen: 0\n  EXPANSION1_ID.ENABLED: 0|' \
    -e 's|^floats:|floats:\n  UIScalePref: 175|' "$CONFIG/kplayerprefs.yaml"

if [ $MONO = 1 ]; then
    # The PC's Mono player, run from a copy of its executable next to steam_appid.txt, so
    # Steamworks does not relaunch it through Steam (with the real settings). Its Managed
    # directory takes the assemblies tools/scripts/setup_player_project.sh patched for the
    # port (SteamManager no longer quits without Steam), plus this tree's runtime and HUD.
    MB=$HOME/Unity/Hub/Editor/6000.3.5f2/Editor/Data/MonoBleedingEdge
    PATCHED=$ROOT/unity/player/Assets/Plugins/Managed
    RUN=$ROOT/build/mono-check
    M=$RUN/OxygenNotIncluded_Data/Managed
    rm -rf "$RUN"
    mkdir -p "$M"
    cp "$GAME/OxygenNotIncluded" "$RUN/"
    for f in UnityPlayer.so libdecor-0.so.0 libdecor-cairo.so; do
        ln -s "$GAME/$f" "$RUN/$f"
    done
    for f in "$GAME"/OxygenNotIncluded_Data/*; do
        [ "$(basename "$f")" = Managed ] || ln -s "$f" "$RUN/OxygenNotIncluded_Data/"
    done
    for f in "$GAME"/OxygenNotIncluded_Data/Managed/*; do
        name=$(basename "$f")
        if [ -e "$PATCHED/$name" ]; then ln -s "$PATCHED/$name" "$M/"; else ln -s "$f" "$M/"; fi
    done
    echo 457140 > "$RUN/steam_appid.txt"
    G=$GAME/OxygenNotIncluded_Data/Managed
    refs=(-r:"$G/mscorlib.dll" -r:"$G/netstandard.dll" -r:"$G/UnityEngine.CoreModule.dll"
        -r:"$G/UnityEngine.InputLegacyModule.dll")
    "$MB/bin/mcs" -target:library -nostdlib -noconfig "${refs[@]}" \
        -out:"$M/OniMobile.Runtime.dll" "$ROOT"/tools/runtime/*.cs
    refs+=(-r:"$G/UnityEngine.UIModule.dll" -r:"$G/UnityEngine.TextRenderingModule.dll"
        -r:"$G/UnityEngine.ScreenCaptureModule.dll" -r:"$G/UnityEngine.UI.dll" -r:"$G/Unity.TextMeshPro.dll"
        -r:"$PATCHED/Assembly-CSharp.dll" -r:"$PATCHED/Assembly-CSharp-firstpass.dll" -r:"$M/OniMobile.Runtime.dll")
    "$MB/bin/mcs" -target:library -nostdlib -noconfig -nowarn:618 "${refs[@]}" \
        -out:"$M/OniMobile.Layout.dll" "$ROOT"/tools/layout/*.cs
else
    RUN=$ROOT/build/linux-swap
fi

cd "$RUN"
status=0
if [ "$STEPS" = play ]; then
    XDG_CONFIG_HOME=$OUT/xdg ONI_MOBILE_UI=1 ./OxygenNotIncluded -screen-width 1600 -screen-height 720 \
        -screen-fullscreen 0 -logFile "$OUT/player.log"
    exit 0
fi
XDG_CONFIG_HOME=$OUT/xdg ONI_MOBILE_UI=1 ONI_MOBILE_PROBE=$OUT ONI_MOBILE_STEPS=$STEPS \
    timeout 300 ./OxygenNotIncluded -screen-width 1600 -screen-height 720 -screen-fullscreen 0 \
    -logFile "$OUT/player.log" || status=$?
grep -E "OniMobile|Exception" "$OUT/player.log" | head -40 || true
ls "$OUT"
exit "$status"
