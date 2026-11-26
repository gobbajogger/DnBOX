#!/usr/bin/env python3
"""Test-only display DMA interception; no production source logic altered."""
from pathlib import Path
import sys
p=Path(sys.argv[1])/'host/x0x_host.c';s=p.read_text()
a='static void lcd_sync(void) {}';assert s.count(a)==1
s=s.replace(a,'''static int home_dma_enabled;
static void home_dma_finish(void);
static void home_dma_emit(uint32_t,uint32_t,uint32_t,uint32_t,const uint16_t *);
static void lcd_sync(void) { if (home_dma_enabled) home_dma_finish(); }''')
a='    blit_px += (uint64_t)w * h;'
assert s.count(a)==1
s=s.replace(a,a+'\n    if (home_dma_enabled) { home_dma_emit(x,y,w,h,px); return; }')
p.write_text(s)
