"""Decode only; does not establish a capture or producer event join."""
import hashlib, math, struct, sys, json
from pathlib import Path
D=Path(__file__).resolve().parent; W=D.parent
sys.path.insert(0,str(W/'restored-harness-inspect/helpers'))
import guarded_replay as g
def decode(r):
 stage,cue,nflags,w0,w8,ordinal,irq,elapsed,l0,r0,l8,r8=r
 assert stage==3 and cue==65536 and elapsed>0 and ordinal<65536
 n=nflags&511; off=nflags>>16
 assert nflags&0xf800==0 and 8<n<=256 and off+n<=512 and off%256==0
 assert nflags&0x600==0x600,'both first-span 909 taps required for attribution'
 assert irq&~2047==0 and (irq&128)!=0
 a=(irq>>8)&1;b=(irq>>9)&1; moved=(irq>>10)&1
 assert moved==(a!=b)
 bits=lambda w: struct.unpack('<f',struct.pack('<I',w))[0]
 dr=[bits(w) for w in (w0,w8)]
 assert all(map(math.isfinite,dr))
 def signed(w):return (w^2147483648)-2147483648
 return dict(embedded_cue_frame=cue,renderer_request=[cue,cue+n],selected_half_renderer_request=[cue-off,cue-off+512],half_ordinal=ordinal,
             cue_offset_in_selected_half=off, masked_dma_entry_pending=irq&255,
             selected_write_half=a, dma_reading_other_half_at_entry=a^1, free_half_observed_after_ack=b,
             endpoint_moved=bool(moved), elapsed_us=elapsed,pre_mix_909_dry_l_cue_and_plus8=dr,
             final_dma_signed_l_r_cue_and_plus8=[[signed(l0),signed(r0)],[signed(l8),signed(r8)]],
             complete_producer_marker_only=True, anchored_to_capture=False,
             caveat='Equal entry/exit half does not rule out an even number of DMA transitions. No DAC read order, no event-phase proof.')
if __name__=='__main__':
 _,p,h,m=sys.argv
 e,s,_sec=g.symbols_and_sections(Path(p))
 assert len(h)==64 and g.sha(e)==h
 r=Path(m).read_bytes()
 assert len(r)==1048576 and 'lab_first_cue' in s
 off=s['lab_first_cue']-g.ELF_RAM
 assert 0<=off<=len(r)-48
 receipt=decode(struct.unpack_from('<12I',r,off))
 print(json.dumps(dict(elf_sha256=h,ram_sha256=hashlib.sha256(r).hexdigest(),**receipt),indent=2))
