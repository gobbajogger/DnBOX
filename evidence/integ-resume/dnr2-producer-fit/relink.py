import hashlib, json, os, pathlib, subprocess, sys
p,f,t=map(pathlib.Path,sys.argv[1:]); sys.path.insert(0,str(f/'tools')); os.environ['JIELI_TOOLCHAIN']=str(t)
import build
stock=f/'build/smk37-elite'; build.BOARD='smk37'; build.MODEL='elite'; build.SUPERJ6_FACTORY='0'; build.OUT=stock
_,objects=build.app_compile_commands([],True,'0')
flags=['-plugin',str(t/'pi32v2/bin/LLVMgold.so'),'-plugin-opt=-inline-threshold=0','-plugin-opt=-inlinehint-threshold=0','-plugin-opt=-inlinecold-threshold=0','-plugin-opt=-enable-tail-merge','-plugin-opt=-tail-merge-size=1','-plugin-opt=-tail-merge-threshold=10000','-plugin-opt=-function-sections','-plugin-opt=-data-sections','-plugin-opt=-fp-contract=off','--sort-section=alignment','--gc-sections','-e','_start','-T',str(stock/'app.ld')]
stock_app=stock/'x0x.bin'
assert hashlib.sha256(stock_app.read_bytes()).hexdigest()=='650b8dced69b28dea1f549fb87fc0610e5afc3b9aec66c85bd5cedc56f246f45'
def relink(label,extra):
 elf=p/(label+'.elf'); cmd=[str(t/'pi32v2/bin/ld'),*flags,*map(str,objects),*extra,'-o',str(elf)]
 out=subprocess.run(cmd,text=True,capture_output=True); print(label,'link exit',out.returncode,out.stderr,flush=True)
 (p/(label+'-link.log')).write_text(out.stdout+out.stderr)
 return elf if out.returncode==0 else None
original=relink('baseline',[])
if original is None:sys.exit(1)
expected=(stock/'x0x.elf').read_bytes(); current=original.read_bytes(); print('baseline ELF hash',hashlib.sha256(current).hexdigest(),flush=True)
if current!=expected:
 for section in ('.text','.ram_text','.c1_text','.data'):
  for label,elf in [('original',stock/'x0x.elf'),('relink',original)]:
   target=p/(label+section+'.bin')
   subprocess.run([str(t/'common/bin/objcopy'),'-O','binary','-j',section,str(elf),str(target)],check=True)
  a=(p/('original'+section+'.bin')).read_bytes(); b=(p/('relink'+section+'.bin')).read_bytes()
  print(section,len(a),hashlib.sha256(a).hexdigest(), 'byte_equal', a==b, flush=True)
  if a!=b: sys.exit(2)
 # Linker symbol metadata need not be byte-identical; loadable sections above must be.
# Size the unchanged core and only its admitted receive/decode/reply paths. No hardware dispatcher or callbacks.
codec=relink('reference-codec-only',['-u','dnr2_decode','-u','dnr2_encode_reply',str(p/'remote_wire-lto.o')])
linked=relink('reference-core-only',['-u','dnr2_receive','-u','dnr2_init',str(p/'remote_core-lto.o'),str(p/'remote_wire-lto.o')])
if linked is not None or codec is not None:
 def app_end(elf):
  import re
  s=subprocess.check_output([str(t/'common/bin/objdump'),'-t',str(elf)],text=True)
  v=lambda name:int(re.search(r'^([0-9a-f]+) .*\s'+name+r'$',s,re.M).group(1),16)
  length=v('_data_load')-build.APP_XIP
  sections=subprocess.check_output(['readelf','-SW',str(elf)],text=True)
  length+=int(re.search(r'\]\s+\.data\s+PROGBITS\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)',sections).group(1),16)
  return (length+3)//4*4+4
 base=app_end(original)
 for label,measured in [('codec-only',codec),('core-only',linked)]:
  if measured is None:continue
  new=app_end(measured)
  print(json.dumps(dict(probe=label,base_application_bytes=base,reference_application_bytes=new,delta_bytes=new-base,reference_xip_free=552672-new,required_xip_free=1024)),flush=True)
