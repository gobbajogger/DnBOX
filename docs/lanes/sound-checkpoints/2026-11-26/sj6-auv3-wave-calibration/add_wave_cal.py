from pathlib import Path
import sys
p=Path(sys.argv[1]);src=p/'main.swift';s=src.read_text()
def rep(x,y):
 global s
 assert s.count(x)==1,(x,s.count(x))
 s=s.replace(x,y,1)
rep(' let compressor=mode.hasPrefix("compressor-")',''' let shapeKey=mode.components(separatedBy:"-").dropFirst().first ?? ""
 let shapeDetents:[String:Float]=["tri":Float(1.0/3.0),"saw":Float(2.0/3.0),"pulse":1]
 let shape=shapeDetents[String(shapeKey)]
 let compressor=mode.hasPrefix("compressor-")''')
rep(' let compFields=mode.split(separator:"-")',' let compFields=mode.split(separator:"-").filter{!shapeDetents.keys.contains(String($0))}')
rep('if compressor {neutral[17]=compValue; neutral[123]=0}','if compressor {neutral[17]=compValue; neutral[123]=0; if let shape {neutral[83]=shape}}')
rep('func compressorCalibration(_ dir:URL) throws {','''// Bare detector receives the already-rendered compressor-off shape, normalized
// only for this calibration. Inlet is a conditional estimate, not an AU tap.
func compressorCalibration(_ dir:URL, _ shapeSource:[[Float]]?=nil) throws {''')
rep('let meters=[0.003,','let meters=[0.003,')
rep('''            src[0][j]=q; src[1][j]=q''','''            if let shapeSource {
                src[0][j]=Float(level*Double(shapeSource[0][j]))
                src[1][j]=Float(level*Double(shapeSource[1][j]))
            } else { src[0][j]=q; src[1][j]=q }''')
rep('''    let payload:[String:Any]=["component":"aufx/dcmp/appl"''','''    let payload:[String:Any]=["input_shape":shapeSource != nil ? "normalized native compressor-off stereo PCM" : "sine", "component":"aufx/dcmp/appl"''')
insert='''
func normalizedCaptureForCalibration(_ dir:URL) throws -> ([[Float]],Double,String) {
    let data=try Data(contentsOf:dir.appendingPathComponent("output.npy"))
    let receipt=try JSONSerialization.jsonObject(with:Data(contentsOf:dir.appendingPathComponent("capture.json"))) as! [String:Any]
    try require(data.count>10 && data.prefix(8).elementsEqual([0x93,0x4e,0x55,0x4d,0x50,0x59,1,0]),"unexpected PCM wrapper")
    let start=10+Int(data[8])+Int(data[9])*256, frames=148684
    try require(data.count-start==frames*2*4,"unexpected PCM length")
    let bytes=data.subdata(in:start..<data.count)
    let hash=digest(bytes)
    try require(hash==receipt["pcm_sha256"] as? String,"off PCM hash mismatch")
    var wave=[[Float](repeating:0,count:frames),[Float](repeating:0,count:frames)]
    for c in 0..<2 {
        for i in 0..<frames {
            let o=start+(c*frames+i)*4
            let bits=UInt32(data[o]) | (UInt32(data[o+1])<<8) | (UInt32(data[o+2])<<16) | (UInt32(data[o+3])<<24)
            wave[c][i]=Float(bitPattern:bits)
        }
    }
    let peak=wave.map{$0[(16384+11025)..<(16384+33075)].map{abs($0)}.max() ?? 0}.max() ?? 0
    try require(peak>0 && peak.isFinite && wave.allSatisfy{$0.allSatisfy(\.isFinite)},"invalid off waveform")
    return (wave.map{$0.map{$0/peak}},Double(peak),hash)
}
func calibrateMeasuredWaveforms(_ dir:URL) throws {
    for key in ["tri","saw","pulse"] {
        let src=dir.appendingPathComponent("native-jupiter-sine-gain-authored-compressor-"+key+"-f1000-off")
        let shape=try normalizedCaptureForCalibration(src)
        let dest=dir.appendingPathComponent("bare-"+key)
        try FileManager.default.createDirectory(at:dest,withIntermediateDirectories:false)
        let manifest:[String:Any]=["off_peak":shape.1,"off_pcm_sha256":shape.2,"source_leaf":src.lastPathComponent]
        try JSONSerialization.data(withJSONObject:manifest,options:[.sortedKeys]).write(to:dest.appendingPathComponent("shape.json"),options:.atomic)
        try compressorCalibration(dest,shape.0)
    }
}
'''
rep('struct CutoffStep: Codable',insert+'\nstruct CutoffStep: Codable')
rep('let profiles: [String:BenchProfile] = [','''let compressorWaveSequence:[(String,String)] = {
    var arms:[(String,String)]=[]
    for k in ["tri","saw","pulse"] {
        for f in ["f125","f250","f375","f500","f750","f1000"] {
            for state in ["off","on"] {arms.append(("compressor-"+k+"-"+f+"-"+state,"authored"))}
        }
        for state in ["on","off"] {arms.append(("compressor-"+k+"-f500-"+state+"-repeat","authored"))}
    }
    return arms
}()
let profiles: [String:BenchProfile] = [
    "jupiter-compressor-waves":BenchProfile(run:capture_gain,sequence:compressorWaveSequence),''')
rep('        guard i<prof.sequence.count else {complete(true,"all \\(i) native renders complete");return}', '''        guard i<prof.sequence.count else {
            if j.profile=="jupiter-compressor-waves" {
                publish("Calibrating bare compressor on measured waveforms")
                self.renderQueue.async {
                    do {
                        try calibrateMeasuredWaveforms(dir)
                        DispatchQueue.main.async {self.complete(true,"all \\(i) waves and matched bare calibrations complete")}
                    } catch {DispatchQueue.main.async {self.complete(false,String(describing:error))}}
                }
            } else {complete(true,"all \\(i) native renders complete")}
            return
        }''')
src.write_text(s)
sub=p/'submit.py';txt=sub.read_text();assert txt.count('"jupiter-compressor"')==1
sub.write_text(txt.replace('"jupiter-compressor"','"jupiter-compressor","jupiter-compressor-waves"'))
