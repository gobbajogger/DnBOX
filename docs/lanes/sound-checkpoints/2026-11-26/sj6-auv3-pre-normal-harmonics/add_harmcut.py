from pathlib import Path
import sys
p=Path(sys.argv[1]);root=p/'main.swift';s=root.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,(a,s.count(a))
 s=s.replace(a,b,1)
rep(' let compressor=mode.hasPrefix("compressor-")',''' let harmonics=mode.hasPrefix("harmonic-")
 let hFields=mode.split(separator:"-")
 let hCutoffs:[String:Float]=["open":1,"c450":.45,"c500":.5,"c625":.625,"c750":.75]
 let hCutoff=harmonics && hFields.count==3 ? hCutoffs[String(hFields[2])] : nil
 try require(!harmonics || hCutoff != nil,"invalid cutoff cue")
 let compressor=mode.hasPrefix("compressor-")'''.replace(':.45',':0.45').replace(':.5',':0.5').replace(':.625',':0.625').replace(':.75',':0.75'))
rep(' || poly || compressor',' || poly || compressor || harmonics')
rep(' let fader=compFader ?? volumeSweep[mode] ??',' let fader=(harmonics ? Float(1):nil) ?? compFader ?? volumeSweep[mode] ??')
rep('if compressor {neutral[17]=compValue; neutral[123]=0; if let shape {neutral[83]=shape}}','''if compressor {neutral[17]=compValue; neutral[123]=0; if let shape {neutral[83]=shape}}
 if harmonics, let hCutoff { neutral[17]=0;neutral[123]=0;neutral[7]=hCutoff;neutral[83]=shape ?? 0 }''')
rep('compressor ? "jupiter-compressor-A4-v1" : (poly ?', 'harmonics ? "jupiter-harmonic-cutoff-A4-v1" : compressor ? "jupiter-compressor-A4-v1" : (poly ?')
rep('let profiles: [String:BenchProfile] = [','''let harmonicCutSequence:[(String,String)]={
    var r:[(String,String)]=[]
    for shape in ["sine","tri"] {
        for name in ["open","c450","c500","c625","c750","c500-repeat","open-repeat"] {
            r.append(("harmonic-"+shape+"-"+name,"authored"))
        }
    }
    return r
}()
let profiles: [String:BenchProfile] = [
    "jupiter-harmonic-cutoff":BenchProfile(run:capture_gain,sequence:harmonicCutSequence),''')
root.write_text(s)
t=p/'submit.py';s=t.read_text();assert s.count('"jupiter-compressor-waves"')==1
t.write_text(s.replace('"jupiter-compressor-waves"','"jupiter-compressor-waves","jupiter-harmonic-cutoff"'))
