from pathlib import Path
import subprocess,struct,sys,json
e,r=sys.argv[1:]; b=Path(r).read_bytes()
def find(key):
 for l in subprocess.check_output(['readelf','-sW',e],text=True).splitlines():
  c=l.split()
  if len(c)>=8 and c[7]==key and c[6]!='UND':return int(c[1],16)-0x01c00000,int(c[2])
a,n=find('gap_worst');assert n==44
half,us,ms,*rs=struct.unpack_from('<11I',b,a)
dbg,n=find('x0x_dbg')
assert struct.unpack_from('<I',b,dbg+8)[0]==us
names=('pre_first_prepare','first_prepare','frame0_queues_arp_boundary_actions','between_span_poll_until_render','after_render_prepare','after_prepare_seq_advance_brk_midi','block_output_master_remainder','pcm_output_scope_meters')
data={k:v/24 for k,v in zip(names,rs)}
print(json.dumps(dict(worst_half=half,worst_us=us,wall_ms=ms,narrow_windows_us=data,total_us=sum(data.values())),indent=2))
