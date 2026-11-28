#!/bin/sh
# App/loader outputs are emulator diagnostics, not update packages for a physical device.
set -eu
[ "$#" = 2 ] || { echo "Usage: $0 SOURCE_DIR elite|pro" >&2; exit 2; }
: "${JIELI_TOOLCHAIN:?Set the pinned toolchain directory}" "${AC79_SDK:?Set the pinned SDK directory}"
case "$2" in elite|pro) ;; *) exit 2 ;; esac
cd "$1"
export MODEL="$2" X0X_UI_SMALL_CANVAS=1 X0X_BASS_PRESET_PREVIEW=1
export X0X_GEN_CACHE="${X0X_GEN_CACHE:-0}"
export X0X_BASS_TAGGED_SOUND=1 X0X_BASS_NOTE_SOUND=0 X0X_D9_TANH_BOOT=1
export X0X_MIAMI_N513_BOOT=1 X0X_CHI_FACTORY_COL=1 X0X_SUPERJ6_FACTORY=0
export X0X_SYNTHONE_PATCH=1 X0X_S1C=1 DNBOX_LCD_CANVAS_48=1 DNBOX_LAB_LEGACY_SEMANTIC_RENDER=1
exec ./build.sh --board smk37
