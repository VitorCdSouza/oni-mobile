#!/usr/bin/env bash
# Copy the game's managed assemblies into the IL2CPP player project
# (unity/player/Assets/Plugins/Managed). The base class library and the
# engine modules are left out: the editor supplies its own copies.
set -euo pipefail

GAME_DATA=${1:-$HOME/.steam/steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DEST=$ROOT/unity/player/Assets/Plugins/Managed

plugin_meta() {  # plugin_meta <assembly name>
    # The game declares global-namespace types (Action, DateTime, EventSystem...) that break
    # package sources compiled against it, so nothing references these assemblies at compile
    # time; they only go into the player build.
    local guid; guid=$(printf '%s' "oni-mobile/$1" | md5sum | cut -c1-32)
    cat > "$DEST/$1.dll.meta" <<EOF
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
  isExplicitlyReferenced: 1
  validateReferences: 0
  platformData:
  - first:
      Any: 
    second:
      enabled: 1
      settings: {}
  - first:
      Editor: Editor
    second:
      enabled: 1
      settings:
        CPU: AnyCPU
        DefaultValueInitialized: true
        OS: AnyOS
  - first:
      Android: Android
    second:
      enabled: 1
      settings:
        CPU: ARM64
  - first:
      Standalone: Linux64
    second:
      enabled: 1
      settings:
        CPU: x86_64
  userData:
  assetBundleName:
  assetBundleVariant:
EOF
}

# From scratch: an assembly the game no longer ships would stay and go into Keep.cs.
rm -rf "$DEST"
mkdir -p "$DEST"
for dll in "$GAME_DATA"/Managed/*.dll; do
    name=$(basename "$dll" .dll)
    case "$name" in
        mscorlib|netstandard|Mono.Security|I18N|I18N.*) continue ;;
        System|System.Core|System.Data|System.Data.*|System.Drawing|System.Xml|System.Xml.*) continue ;;
        System.ComponentModel.*|System.Configuration|System.EnterpriseServices|System.Net.Http) continue ;;
        System.Numerics|System.Runtime|System.Runtime.Serialization|System.Security) continue ;;
        System.ServiceModel.*|System.Transactions|System.IO.Compression|System.IO.Compression.*) continue ;;
        UnityEngine|UnityEngine.*Module) continue ;;
        # Editor-side packages that only ship for the original project's tooling.
        Unity.MemoryProfiler|Unity.Multiplayer.Center.Common) continue ;;
    esac
    cp "$dll" "$DEST/"
    plugin_meta "$name"
done
ls "$DEST"

# Nothing in the project uses these assemblies, and the C# compiler drops unused references,
# so Keep.cs names one type of each: that makes Unity carry them into the player build.
MB=$(ls -d "$HOME"/Unity/Hub/Editor/6000.3.5f2/Editor/Data/MonoBleedingEdge)
CECIL=$(dirname "$MB")/il2cpp/build/deploy/Mono.Cecil.dll
mkdir -p "$ROOT/build/picktypes"
cp "$CECIL" "${CECIL%.dll}.Rocks.dll" "$ROOT/build/picktypes/"
"$MB/bin/mcs" -sdk:4.5 -out:"$ROOT/build/picktypes/PickTypes.exe" -r:"$CECIL" \
    -r:"$MB/lib/mono/4.5/Facades/netstandard.dll" -r:"$MB/lib/mono/4.5/Facades/System.Runtime.dll" \
    "$ROOT/tools/picktypes/PickTypes.cs"
for tool in PatchAot PatchAndroid; do
    "$MB/bin/mcs" -sdk:4.5 -out:"$ROOT/build/picktypes/$tool.exe" -r:"$CECIL" -r:"${CECIL%.dll}.Rocks.dll" \
        -r:"$MB/lib/mono/4.5/Facades/netstandard.dll" -r:"$MB/lib/mono/4.5/Facades/System.Runtime.dll" \
        "$ROOT/tools/picktypes/$tool.cs" "$ROOT/tools/picktypes/NameResolver.cs"
done

# IL2CPP has no Reflection.Emit: make Newtonsoft.Json use its late-bound code paths.
"$MB/bin/mono" "$ROOT/build/picktypes/PatchAot.exe" "$GAME_DATA/Managed/Newtonsoft.Json.dll" \
    "$DEST/Newtonsoft.Json.dll" "$GAME_DATA/Managed"
# Touch controls and Android defaults (tools/runtime), built against the game's own engine assemblies.
M=$GAME_DATA/Managed
"$MB/bin/mcs" -target:library -nostdlib -noconfig -r:"$M/mscorlib.dll" -r:"$M/netstandard.dll" \
    -r:"$M/UnityEngine.CoreModule.dll" -r:"$M/UnityEngine.InputLegacyModule.dll" \
    -out:"$DEST/OniMobile.Runtime.dll" "$ROOT"/tools/runtime/*.cs
plugin_meta OniMobile.Runtime
# StreamingAssets, Steam, native callbacks and touch input on Android (see tools/picktypes/PatchAndroid.cs).
for asm in Assembly-CSharp Assembly-CSharp-firstpass FMODUnity UnityEngine.UI; do
    "$MB/bin/mono" "$ROOT/build/picktypes/PatchAndroid.exe" "$GAME_DATA/Managed/$asm.dll" \
        "$DEST/$asm.dll" "$GAME_DATA/Managed" "$DEST/OniMobile.Runtime.dll"
done
# The phone HUD (tools/layout), built against the patched game assemblies. OniMobile.Runtime
# loads it by name, and link.xml keeps it whole.
"$MB/bin/mcs" -target:library -nostdlib -noconfig -nowarn:618 -r:"$M/mscorlib.dll" -r:"$M/netstandard.dll" \
    -r:"$M/UnityEngine.CoreModule.dll" -r:"$M/UnityEngine.InputLegacyModule.dll" -r:"$M/UnityEngine.UIModule.dll" \
    -r:"$M/UnityEngine.TextRenderingModule.dll" -r:"$M/UnityEngine.ScreenCaptureModule.dll" \
    -r:"$DEST/UnityEngine.UI.dll" -r:"$DEST/Unity.TextMeshPro.dll" -r:"$DEST/OniMobile.Runtime.dll" \
    -r:"$DEST/Assembly-CSharp.dll" -r:"$DEST/Assembly-CSharp-firstpass.dll" \
    -out:"$DEST/OniMobile.Layout.dll" "$ROOT"/tools/layout/*.cs
plugin_meta OniMobile.Layout
{
    echo '// Generated by tools/scripts/setup_player_project.sh. Do not edit.'
    echo '// Names one type from each game assembly so the compiler references them all and'
    echo '// Unity includes them in the player build.'
    echo 'namespace OniMobile'
    echo '{'
    echo '    internal static class Keep'
    echo '    {'
    echo '        internal static readonly global::System.Type[] Types ='
    echo '        {'
    # The BCL facades clash with the editor's own copies when named directly; the game
    # assemblies reference them, which is enough to bring them along.
    "$MB/bin/mono" "$ROOT/build/picktypes/PickTypes.exe" "$DEST"/*.dll | grep -vE '// System\.(Memory|Runtime\.CompilerServices\.Unsafe)$'
    echo '        };'
    echo '    }'
    echo '}'
} > "$ROOT/unity/player/Assets/GameRefs/Keep.cs"
