from pathlib import Path
import sys
p=Path(sys.argv[1]); root=p/'main.swift'
all_src=root.read_text()
start=all_src.index('func capture_gain(')
end=all_src.index('\nstruct CutoffStep: Codable',start)
head,s,queue=all_src[:start],all_src[start:end],all_src[end:]
def one(a,b):
 global s
 if s.count(a)!=1: raise ValueError((a,s.count(a)))
 s=s.replace(a,b,1)
one(' let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode)', ''' let polyModes=["poly-held","poly-additions","poly-all"]
 let poly=polyModes.contains(mode)
 let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode) || poly''')
one('((mode=="osc2-only" || velocitySweep[mode] != nil)', '((mode=="osc2-only" || poly || velocitySweep[mode] != nil)')
one('22:0,23:0,24:0,28:0,32:1,33:0','22:0,23:0,24:0,28:0,32:poly ? 0:1,33:0')
one(''' let ev:[[String:Any]] = [
   ["frame":16384,"note":69,"velocity":midiVelocity,"action":"on"],
   ["frame":60484,"note":69,"velocity":0,"action":"off"]
 ]''',''' let soloEvents:[[String:Any]] = [
   ["frame":16384,"note":69,"velocity":midiVelocity,"action":"on"],
   ["frame":60484,"note":69,"velocity":0,"action":"off"]
 ]
 let polyCue:[(Int,Int,Bool)] = [
   (16384,57,true), (28672,45,true), (28672,55,true),
   (40960,60,true), (40960,73,true), (40960,82,true),
   (57344,60,false), (57344,73,false), (57344,82,false),
   (69632,45,false), (69632,55,false), (81920,57,false)
 ]
 let ev:[[String:Any]]=poly ? polyCue.map { frame,key,on in
   ["frame":frame,"note":key,"velocity":on ? midiVelocity : 0,"action":on ? "on":"off",
    "send":mode=="poly-all" || (mode=="poly-held" ? key==57 : key != 57)]
 } : soloEvents''')
one('    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])','''    let row=ev[ei]
    if row["send"] as? Bool != false {
        note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    }''')
one('swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1"', 'poly ? "jupiter-poly-held-v1" : (swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1")')
assert queue.count('    "jupiter-response": BenchProfile')==1
queue=queue.replace('    "jupiter-response": BenchProfile', '''    "jupiter-poly": BenchProfile(run:capture_gain,sequence:[
        ("poly-held","authored"),("poly-additions","authored"),("poly-all","authored"),
        ("poly-held-repeat","authored"),("poly-all-repeat","authored")]),
    "jupiter-response": BenchProfile''')
root.write_text(head+s+queue)
(p/'capture_gain.swift').write_text(s)
(p/'queue.swift').write_text(queue.lstrip('\n'))
submit=p/'submit.py'; old=submit.read_text(); assert old.count('"jupiter-response"')==1
submit.write_text(old.replace('"jupiter-response"','"jupiter-response","jupiter-poly"'))
