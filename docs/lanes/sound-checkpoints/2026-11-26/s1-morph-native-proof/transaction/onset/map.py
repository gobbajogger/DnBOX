import subprocess,sys
sym={}
for line in subprocess.check_output(['readelf','-sW',sys.argv[1]],text=True).splitlines():
 c=line.split()
 if len(c)>=8 and c[6]!='UND':
  try:sym[c[7]]=(int(c[1],16),int(c[2]))
  except ValueError: pass
for k in ('x0x_dbg','audio_xruns','eng_stage','eng_job_arg','dry_l','dry_r','rev','dly','d909','seq','ui','fm1_in','d8_frame','eng_hit_ms','live_key_r','live_key_w','d9_verify_good','d9_verify_state','d9_verify_pcm'):
 if k in sym:
  addr,size=sym[k]
  # Use full audio buses and full HAL/sequencer. 909 specific state proof lives in separate native shadow.
  if k=='d909':continue
  print(k,hex(addr)[2:],size)
