"""Parse retained existing 16-byte marker in a native RAM dump, not PCM."""
import json, math, struct, sys
from pathlib import Path
def decode(words):
    w,d9,d8,dma=words
    assert w&0xf80000fb==0xc0000003
    off=256*((w>>2)&1)
    a=(w>>8)&1;b=(w>>9)&1;ordinal=(w>>11)&65535
    f=lambda x:struct.unpack('<f',struct.pack('<I',x))[0]
    dat=[f(d9),f(d8)]
    assert all(map(math.isfinite,dat))
    return dict(cue_frame=65536,sampled_render_frame=65544,render_call=[65536,65792],
                selected_half_renderer_request=[65536-off,66048-off],half_ordinal=ordinal,
                initial_free_half=a,final_free_half=b,sampled_dma_endpoint_moved=a!=b,
                tap_words=[f'{x:08x}' for x in words[1:]],tap_909_pre_shape_l=dat[0],
                tap_808_post_shape_pre_mix_l=dat[1],tap_dma_final_int32_l=((dma^0x80000000)-0x80000000),
                armed_before_embedded_cue=True,complete_producer_marker_only=True,
                applied_external_event_anchor=False,dac_capture_anchor=False)
if __name__=='__main__':
    cel,sha,ram=sys.argv[1:]
    path=Path(__file__).resolve()
    sys.path.insert(0,str(path.parents[1]/'restored-harness-inspect/helpers'))
    import guarded_replay as g
    data,s,_=g.symbols_and_sections(Path(cel))
    assert g.sha(data)==sha
    r=Path(ram).read_bytes(); assert len(r)==1048576 and 'd9_test_timeline' in s
    off=s['d9_test_timeline']-g.ELF_RAM
    assert 0<=off<=len(r)-16
    print(json.dumps(dict(elf_sha256=sha,ram_sha256=g.sha(r),**decode(struct.unpack_from('<4I',r,off))),indent=2))
