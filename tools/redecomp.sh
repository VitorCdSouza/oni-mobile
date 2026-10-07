#!/usr/bin/env bash
# Regenerates the layout headers, applies the struct layouts to the Ghidra project and exports
# the decompile again into sim/decomp.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
export JAVA_HOME="$(ls -d "$root"/tools/jdk-21*)"
export PATH="$JAVA_HOME/bin:$PATH"
python3 -I "$root/sim/layout/gen_layout.py" "$root/sim/layout/layout.txt" \
    "$root/sim/layout/ghidra_types.h" "$root/sim/port/src/layout_checks.h"
rm -rf "$root/sim/decomp/functions"
"$root"/tools/ghidra_*/support/analyzeHeadless "$root/sim/ghidra-proj" simdll \
    -process libSimDLL.x86_64.so -noanalysis -scriptPath "$root/tools/scripts" \
    -postScript ApplyTypes.java "$root/sim/layout/ghidra_types.h" \
    -postScript ExportDecomp.java "$root/sim/decomp" > "$root/sim/ghidra.log" 2>&1 || { tail -20 "$root/sim/ghidra.log"; exit 1; }
grep -E 'ApplyTypes:|ERROR' "$root/sim/ghidra.log" | head -20
