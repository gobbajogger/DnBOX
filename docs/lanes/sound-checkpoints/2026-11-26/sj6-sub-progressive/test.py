from pathlib import Path
import ctypes as C,sys,struct,json,math,random,hashlib
def raw(x):return b'\0'+struct.pack('<'+'h'*len(x),*x)
def block(x):
 if min(x)==max(x):return b'\x01'+struct.pack('<h',x[0])
 z=[c-2*b+a for a,b,c in zip(x,x[1:],x[2:])]
 z=[2*d if d>=0 else -2*d-1 for d in z]
 w=max(z,default=0).bit_length()
 if 6+((len(z)*w+7)//8)>=1+len(x)*2:return raw(x)
 u=sum(d<<(i*w) for i,d in enumerate(z))
 return b'\x02'+struct.pack('<hhB',*x[:2],w)+u.to_bytes((len(z)*w+7)//8,'little')
d=C.CDLL(sys.argv[1]); d.decode_exact.argtypes=[C.c_char_p,C.c_size_t,C.POINTER(C.c_int16),C.c_uint]
rng=random.Random(216);N=256*96
profiles={
 'zero':[0]*N,
 'low_sine_exact_PCM16':[round(26000*math.sin(2*math.pi*65.4064*i/44100)) for i in range(N)],
 'multi_sine_exact_PCM16':[round(9500*math.sin(2*math.pi*261.626*i/44100)+8000*math.sin(2*math.pi*523.252*i/44100+.28)+3500*math.sin(2*math.pi*785.*i/44100+.7)) for i in range(N)],
 'broadband_iid_PCM16':[rng.randrange(-32768,32768) for i in range(N)]}
results={}
for n in (64,128,256):
 for label,x in profiles.items():
  sizes=0;modes=[0]*3;blocks=N//n;bad=0;h=hashlib.sha256()
  for i in range(0,N,n):
   v=x[i:i+n];enc=block(v);y=(C.c_int16*n)()
   assert d.decode_exact(enc,len(enc),y,n)==1 and list(y)==v
   assert d.decode_exact(enc,len(enc)-1,y,n)==0
   sizes+=len(enc)+2;modes[enc[0]]+=1;bad+=1
   h.update(enc)
  sizes+=4*math.ceil(blocks/16) # per restart group locator
  results[f'{n}:{label}']=dict(reached=N, rejected_truncated_blocks=bad, payload_modes=modes,
   encoded_bytes_including_header_and_index=sizes, compression_ratio=N*2/sizes, hash=h.hexdigest())
class Window(C.Structure):
 _fields_=[('row',C.POINTER(C.c_int16)*2),('first',C.c_uint*2),('count',C.c_uint*2)]
x=(C.c_int16*4)(-4,9,-16,81);y=(C.c_int16*4)(17,-42,81,-90)
w=Window((C.POINTER(C.c_int16)*2)(x,y),(C.c_uint*2)(4,8),(C.c_uint*2)(4,4))
a,b=C.c_int16(45),C.c_int16(91)
assert d.exact_ready_pair(C.byref(w),7,C.byref(a),C.byref(b))==1 and (a.value,b.value)==(81,17)
assert d.exact_ready_pair(C.byref(w),11,C.byref(a),C.byref(b))==0 and (a.value,b.value)==(81,17)
ref=json.loads(Path('sj6-sub-current-cursor/size.json').read_text())
pcm=ref['decoded_PCM16_mono_total']; frames=pcm//2;pool=112*1024
bound={}
for n in (64,128,256):
 blocks=math.ceil(frames/n)
 index_floor=4*math.ceil(blocks/16)
 # Ignoring earlier source-region padding, storage page rounding and reserved pool safely favors the candidate.
 minbytes=3*blocks+index_floor+288
 zero_only_minbytes=5*blocks+index_floor+288
 bound[str(n)]=dict(blocks_optimistic=blocks, ideal_per_block_seed_bytes=3,
 min_any_design_with_per_block_seed_and_16_restart_index_B=minbytes, this_format_zero_only_B=zero_only_minbytes,
 bit_budget_per_input_frame=max(0,pool-zero_only_minbytes)*8/frames,
 cache_bytes_one_voice_pair=4*n, cache_bytes_six_voice=24*n, cache_bytes_two_six_voice=48*n)
print(json.dumps(dict(input='independently authored synthetic PCM16 only',all_words_exact=12*N,
 pool_bytes_best_case=pool, decoded_pcm16=pcm, required_ratio=pcm/pool,
 codec='finite second-difference pack or raw/constant; frame count trust from bound owner',results=results,
 candidate_storage_floor=bound, ready_pair_hit_and_miss_atomic=True,
 no_timing_or_real_asset_compression_claim=True),indent=2))
