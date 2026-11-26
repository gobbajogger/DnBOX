#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D"); cd "$R"
P=rehydrated-reverb/dual-room/tree
for model in elite pro;do
 E="$P/labout/$model/never-install.elf"; sha256sum "$E"
 nm -aS "$E" 2>/dev/null | awk '$4 ~ /^(cv_px|cv_lcd_txbuf|main_tileh)$/'
done
P=$P/firmware/src
rg -n '\bcv_px\b|\bcv_begin\(' "$P"
rg -n 'CV_ROWS|MAIN_ROWS|BAND_H|X0X_LCD_MAX_DMA_BYTES|smk_apc_usb_tick|cv_lcd_input\(|cv_lcd_maybe_input\(|blit_hash\[i\] = 0|project_revision\+\+' "$P"/{app/ui.c,app/main_fm1.c,app/lcd_tiles.h,gfx.c}
