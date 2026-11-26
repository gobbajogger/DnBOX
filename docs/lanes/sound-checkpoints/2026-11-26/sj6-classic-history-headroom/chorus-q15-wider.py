import pathlib,sys
p=pathlib.Path(sys.argv[1]); s=p.read_text()
pairs=[('/ 32768.0f;','/ 4096.0f;'),('clampf(in * 32767, -32768, 32767)','clampf(in * 4096.f, -32768, 32767)')]
for a,b in pairs:
 assert s.count(a)==1
 s=s.replace(a,b)
p.unlink()
p.write_text(s)
