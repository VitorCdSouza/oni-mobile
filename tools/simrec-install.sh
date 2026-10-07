#!/usr/bin/env bash
# Installs or removes the SimDLL recording shim in the Steam copy of the game.
#   tools/simrec-install.sh install | uninstall | status
set -euo pipefail

plugins="${ONI_PLUGINS:-$HOME/.local/share/Steam/steamapps/common/OxygenNotIncluded/OxygenNotIncluded_Data/Plugins}"
shim="$(cd "$(dirname "$0")/.." && pwd)/sim/build/libSimDLL.so"

case "${1:-status}" in
install)
    if [[ -f "$plugins/libSimDLL_orig.so" ]]; then
        echo "already installed"; exit 0
    fi
    mv "$plugins/libSimDLL.so" "$plugins/libSimDLL_orig.so"
    cp "$shim" "$plugins/libSimDLL.so"
    echo "installed; recordings go to ${ONI_SIMREC_DIR:-$HOME/oni-simrec}"
    ;;
uninstall)
    if [[ ! -f "$plugins/libSimDLL_orig.so" ]]; then
        echo "not installed"; exit 0
    fi
    mv -f "$plugins/libSimDLL_orig.so" "$plugins/libSimDLL.so"
    echo "original library restored"
    ;;
status)
    [[ -f "$plugins/libSimDLL_orig.so" ]] && echo "installed" || echo "not installed"
    ;;
*)
    echo "usage: $0 install|uninstall|status" >&2; exit 2
    ;;
esac
