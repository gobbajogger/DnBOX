"""Terminate single disposable emulator arm early on an emitted fatal witness."""
import re,subprocess,sys,time
from pathlib import Path
bad=re.compile(rb"^\s*(?:halt(?:ed)?:|EXCEPTION|TRAP |PANIC|illegal opcode|fatal:|chip reset at)|panicked at|^flash: [1-9][0-9]* erases / programs",re.I)
log=Path("run.log")
with log.open("xb") as out, log.open("rb") as read:
 p=subprocess.Popen(sys.argv[1:], stdout=out, stderr=subprocess.STDOUT)
 until=time.monotonic()+300
 def stop(why):
  if p.poll() is None:
   p.terminate()
   try:p.wait(3)
   except subprocess.TimeoutExpired:p.kill();p.wait()
  print("watchdog stops arm:",why,file=sys.stderr)
  sys.exit(97)
 while p.poll() is None:
  for line in read:
   if bad.search(line):stop(line.decode(errors="replace").strip())
  if time.monotonic()>until:stop("wall timeout")
  time.sleep(.05)
 for line in read:
  if bad.search(line):stop(line.decode(errors="replace").strip())
 sys.exit(p.returncode)
