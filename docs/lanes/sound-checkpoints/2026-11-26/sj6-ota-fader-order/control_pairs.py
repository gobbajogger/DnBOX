from pathlib import Path
import sys
d,t=map(Path,sys.argv[1:])
ss=(d/'normal-static.private-output/2-dry.private.txt').read_text().splitlines()
n=int(ss[1]);w={int(z.split()[0]):z.split()[1] for z in ss[2:2+n]};ew=ss[2+n:]
for k,edits in [
 ('a-full',{202:0}),('a-half',{202:0,1:64/127}),
 ('a-zero',{202:0,1:0}),('b-full',{202:0,1:0,3:1}),
 ('b-half',{202:0,1:0,3:64/127}),('both',{202:0,1:64/127,3:64/127})
]:
 z=w|{a:str(b) for a,b in edits.items()}
 (t/(k+'.private.txt')).write_text('\n'.join(ss[:2]+[f'{a} {b}' for a,b in z.items()]+ew)+'\n')
