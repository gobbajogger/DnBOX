from pathlib import Path
import sys
p=Path(sys.argv[1]);root=p/'main.swift'
ss=root.read_text()
i=ss.index('func capture_gain('); j=ss.index('\nstruct CutoffStep:',i)
head,s,queue=ss[:i],ss[i:j],ss[j:]
def rep(a,b):
 global s
 assert s.count(a)==1,(a,s.count(a))
 s=s.replace(a,b,1)
rep(' let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode) || poly',
''' let compressor=mode.hasPrefix("compressor-")
 let compFields=mode.split(separator:"-")
 let compValue:Float=compressor && compFields.last=="on" ? 0.02 : 0
 let compFaders:[String:Float]=["f0625":0.0625,"f125":0.125,"f250":0.25,"f375":0.375,"f500":0.5,"f750":0.75,"f1000":1]
 let compFader=compressor && compFields.count==3 ? compFaders[String(compFields[1])] : nil
 try require(!compressor || (compFader != nil && (compFields.last=="on" || compFields.last=="off")),"invalid compressor arm")
 let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode) || poly || compressor''')
rep(' let fader=volumeSweep[mode] ??',' let fader=compFader ?? volumeSweep[mode] ??')
rep(' let neutral:[AUParameterAddress:Float] =',' var neutral:[AUParameterAddress:Float] =')
rep(' for (id,value) in neutral.sorted',' if compressor {neutral[17]=compValue; neutral[123]=0}\n for (id,value) in neutral.sorted')
rep('poly ? "jupiter-poly-held-v1" : (swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1")','compressor ? "jupiter-compressor-A4-v1" : (poly ? "jupiter-poly-held-v1" : (swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1"))')
profile_decl='let profiles: [String:BenchProfile] = ['
assert queue.count(profile_decl)==1
queue=queue.replace(profile_decl, '''let compressorSequence: [(String,String)] = {
    var arms:[(String,String)]=[]
    for f in ["f0625","f125","f250","f375","f500","f750","f1000"] {
        arms.append(("compressor-"+f+"-off","authored"))
        arms.append(("compressor-"+f+"-on","authored"))
    }
    for f in ["f500","f250"] {
        arms.append(("compressor-"+f+"-on-repeat","authored"))
        arms.append(("compressor-"+f+"-off-repeat","authored"))
    }
    return arms
}()
'''+profile_decl)
key='    "jupiter-poly": BenchProfile'
assert queue.count(key)==1
queue=queue.replace(key,'''    "jupiter-compressor": BenchProfile(run:capture_gain,sequence:compressorSequence),
'''+key)
key='            next(j,prof,0,dir)\n            return'
assert queue.count(key)==1
queue=queue.replace(key,'''            if j.profile=="jupiter-compressor" {
                publish("Calibrating Apple Dynamics Processor")
                self.renderQueue.async {
                    do {
                        try compressorCalibration(dir)
                        DispatchQueue.main.async { self.publish("Apple calibration complete"); self.next(j,prof,0,dir) }
                    } catch { DispatchQueue.main.async { self.complete(false,String(describing:error)) } }
                }
            } else { next(j,prof,0,dir) }
            return''')
root.write_text(head+s+'\n'+(p/'bare.swift').read_text()+queue)
sub=p/'submit.py';s=sub.read_text();assert s.count('"jupiter-poly"')==1
sub.write_text(s.replace('"jupiter-poly"','"jupiter-poly","jupiter-compressor"'))
