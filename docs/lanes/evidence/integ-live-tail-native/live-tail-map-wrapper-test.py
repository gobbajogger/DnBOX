from pathlib import Path
import tempfile,subprocess,os,json
base=Path(__file__).resolve().parent
old=base/'combined-link-map-wrapper.py';new=base/'live-tail-map-wrapper.py'
results=[]
for wrapper,status,expected,receipt in [(old,1,0,True),(new,1,1,False),(new,0,0,True)]:
 with tempfile.TemporaryDirectory(prefix='map-wrapper-contract-')as tmp:
  r=Path(tmp);(r/'tools').mkdir();out=r/'build/smk37-elite';out.mkdir(parents=True);(out/'x0x.elf').write_bytes(b'fixed synthetic ELF')
  (r/'tools/build.py').write_text("from pathlib import Path\nBOARD='smk37'\nMODEL='elite'\nOUT=Path.cwd()/'build/smk37-elite'\ndef tc(*args,**kw):\n if '-Map' in args: Path(args[args.index('-Map')+1]).write_text('synthetic map')\ndef main():\n tc('pi32v2/bin/ld','-o',OUT/'x0x.elf')\n return "+str(status)+"\n")
  run=subprocess.run(['python',str(wrapper)],cwd=r,env={**os.environ,'MODEL':'elite'},capture_output=True,text=True)
  assert run.returncode==expected,(wrapper.name,status,run.stderr)
  assert (out/'map-receipt.json').exists()==receipt
  results.append({'wrapper':wrapper.name,'buildReturn':status,'exit':run.returncode,'receipt':receipt,'scope':'synthetic wrapper control flow only'})
print(json.dumps({'cases':results,'result':'PASS'},indent=2))
