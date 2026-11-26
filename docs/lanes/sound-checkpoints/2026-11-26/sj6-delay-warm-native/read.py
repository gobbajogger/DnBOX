import struct,pathlib,json,sys
keys='edit_ticks hwcounter stride filled phase pos ramp words_fingerprint render480_ticks nonzero_frames'.split()
r={}
for a in sys.argv[1:]:
 raw=pathlib.Path(a+'.ram').read_bytes()
 assert len(raw)==1048576
 head=struct.unpack_from('<4I',raw,32768); assert head[:2]==(0x534a3652,0x600d600d) and head[2]>0
 assert raw[:4] != bytes.fromhex('52364a53')
 logs=pathlib.Path(a+'.log').read_text()
 assert 'panel.ppm at 1600 ms' in logs and 'skipped 0 cycles' in logs and 'flash: 0 erases / programs' in logs
 assert pathlib.Path(a+'-nor.bin').read_bytes()==pathlib.Path(a+'-nor-pristine').read_bytes()
 rr=[]
 for i,v in enumerate(struct.iter_unpack('<10I',raw[32784:32784+18*40])):
  row=dict(zip(keys,v));row.update(pass_index=i//6,case_index=i%6,edit_us=v[0]/24,render480_us=v[8]/24)
  rr.append(row)
 r[a]=rr
if 'opt' in r and 'new' in r:
 assert all(all(a[k]==b[k] for k in keys[2:8]+keys[9:]) for a,b in zip(r['new'],r['opt']))
print(json.dumps(r,indent=2))
