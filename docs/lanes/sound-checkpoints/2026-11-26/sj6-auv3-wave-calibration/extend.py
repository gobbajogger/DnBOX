from pathlib import Path
import sys
p=Path(sys.argv[1]);f=p/'main.swift';s=f.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,(a,s.count(a))
 s=s.replace(a,b,1)
rep('func compressorCalibration(_ dir:URL, _ shapeSource:[[Float]]?=nil) throws {','func compressorCalibration(_ dir:URL, _ shapeSource:[[Float]]?=nil, high:Bool=false) throws {')
rep('let meters=[0.003,','let standard=[0.003,')
rep('    for (index,level) in meters.enumerated() {','    let meters=high ? [0.53,0.69,0.79,0.9,0.93,0.96,0.99,1.02,1.06,1.12,1.2,1.3,0.96,1.06,1.12] : standard\n    for (index,level) in meters.enumerated() {')
rep('let profiles: [String:BenchProfile] = [','let profiles: [String:BenchProfile] = [\n    "jupiter-saw-range": BenchProfile(run:capture_gain,sequence:[]),')
rep('            if j.profile=="jupiter-compressor" {','''            if j.profile=="jupiter-saw-range" {
                publish("Calibrating measured Saw above the original sweep ceiling")
                self.renderQueue.async {
                    do {
                        let held=try normalizedCaptureForCalibration(self.root.appendingPathComponent("reference"))
                        let receipt:[String:Any]=["off_peak":held.1,"off_pcm_sha256":held.2]
                        try JSONSerialization.data(withJSONObject:receipt,options:[.sortedKeys]).write(to:dir.appendingPathComponent("shape.json"),options:.atomic)
                        try compressorCalibration(dir,held.0,high:true)
                        DispatchQueue.main.async {self.complete(true,"higher Saw calibration complete")}
                    } catch { DispatchQueue.main.async {self.complete(false,String(describing:error))} }
                }
            } else if j.profile=="jupiter-compressor" {''')
f.write_text(s)
t=p/'submit.py';s=t.read_text();assert s.count('"jupiter-compressor-waves"')==1
t.write_text(s.replace('"jupiter-compressor-waves"','"jupiter-compressor-waves","jupiter-saw-range"'))
