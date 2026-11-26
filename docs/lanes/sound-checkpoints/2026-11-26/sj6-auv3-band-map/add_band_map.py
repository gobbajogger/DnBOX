from pathlib import Path
import sys
p=Path(sys.argv[1]); f=p/'main.swift';s=f.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,(a,s.count(a))
 s=s.replace(a,b,1)
rep(' let harmonics=mode.hasPrefix("harmonic-")',''' let bandMap=mode.hasPrefix("band-")
 let bandFields=mode.split(separator:"-")
 let bandNote=bandMap && bandFields.count==3 ? Int(bandFields[2]):nil
 try require(!bandMap || (bandNote != nil && (0...127).contains(bandNote!)), "invalid band cue")
 let harmonics=mode.hasPrefix("harmonic-")''')
rep(' || compressor || harmonics',' || compressor || harmonics || bandMap')
rep('(harmonics ? Float(1):nil)','((harmonics || bandMap) ? Float(1):nil)')
rep('''   ["frame":16384,"note":69,"velocity":midiVelocity,"action":"on"],
   ["frame":60484,"note":69,"velocity":0,"action":"off"]''','''   ["frame":16384,"note":bandNote ?? 69,"velocity":midiVelocity,"action":"on"],
   ["frame":60484,"note":bandNote ?? 69,"velocity":0,"action":"off"]''')
rep('if harmonics, let hCutoff { neutral[17]=0;neutral[123]=0;neutral[7]=hCutoff;neutral[83]=shape ?? 0 }','''if harmonics, let hCutoff { neutral[17]=0;neutral[123]=0;neutral[7]=hCutoff;neutral[83]=shape ?? 0 }
 if bandMap {neutral[17]=0;neutral[123]=0;neutral[7]=1;neutral[83]=shape ?? 0}''')
rep('harmonics ? "jupiter-harmonic-cutoff-A4-v1" : compressor ?', 'bandMap ? "jupiter-band-map-v1" : harmonics ? "jupiter-harmonic-cutoff-A4-v1" : compressor ?')
rep('let profiles: [String:BenchProfile] = [','''let bandMapSequence:[(String,String)]={
    var rows:[(String,String)]=[]
    for shape in ["sine","tri"] {
        for key in ["57","69","81","93","105","117","45","64","76","88","100","112","69-repeat"] {
            rows.append(("band-"+shape+"-"+key,"authored"))
        }
    }
    return rows
}()
let profiles: [String:BenchProfile] = [
    "jupiter-band-map":BenchProfile(run:capture_gain,sequence:bandMapSequence),''')
f.write_text(s)
t=p/'submit.py';s=t.read_text()
assert s.count('"jupiter-harmonic-cutoff"')==1;t.write_text(s.replace('"jupiter-harmonic-cutoff"','"jupiter-harmonic-cutoff","jupiter-band-map"'))
