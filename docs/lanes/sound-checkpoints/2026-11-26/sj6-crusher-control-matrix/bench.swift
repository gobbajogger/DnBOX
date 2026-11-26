import AppKit
// Native Apple leaf capture only. No SuperJ6 install, device I/O or plugin load.
import AVFoundation
import AudioToolbox
import CryptoKit
import Foundation

enum Failed: Error { case usage, check(String) }
func require(_ pass: Bool, _ what: String) throws {
    if !pass { throw Failed.check(what) }
}
func pcmBytes(_ x: [[Float]]) -> Data {
    var data = Data()
    for channel in x {
        for value in channel {
            var word = value.bitPattern.littleEndian
            withUnsafeBytes(of: &word) { data.append(contentsOf: $0) }
        }
    }
    return data
}
func digest(_ data: Data) -> String {
    SHA256.hash(data: data).map { String(format: "%02x", Int($0)) }.joined()
}
func npy(_ bytes: Data, frames: Int) -> Data {
    let head = "{'descr': '<f4', 'fortran_order': False, 'shape': (2, \(frames)), }"
    let padding = (16 - (10 + head.utf8.count + 1) % 16) % 16
    let hdr = Data((head + String(repeating: " ", count: padding) + "\n").utf8)
    var result = Data([0x93, 0x4e, 0x55, 0x4d, 0x50, 0x59, 0x01, 0x00])
    var count = UInt16(hdr.count).littleEndian
    withUnsafeBytes(of: &count) { result.append(contentsOf: $0) }
    result.append(hdr)
    result.append(bytes)
    return result
}
func fourCC(_ x: UInt32) -> String {
    String(bytes: [24, 16, 8, 0].map { UInt8(truncatingIfNeeded: x >> $0) }, encoding: .ascii) ?? String(x)
}
struct Capture {
    let y: [[Float]]
    let blocks: [Int]
}
func render(_ input: [[Float]], rate: Double, effect: AVAudioUnitEffect?) throws -> Capture {
    let engine = AVAudioEngine()
    let player = AVAudioPlayerNode()
    let quantum = 128
    guard let format = AVAudioFormat(commonFormat: .pcmFormatFloat32, sampleRate: rate,
                                     channels: 2, interleaved: false),
          let source = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: AVAudioFrameCount(input[0].count)),
          let destination = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: AVAudioFrameCount(quantum)),
          let sourceChannels = source.floatChannelData else { throw Failed.check("could not allocate PCM") }
    engine.attach(player)
    if let node = effect {
        engine.attach(node)
        engine.connect(player, to: node, format: format)
        engine.connect(node, to: engine.mainMixerNode, format: format)
    } else {
        engine.connect(player, to: engine.mainMixerNode, format: format)
    }
    engine.mainMixerNode.outputVolume = 1
    source.frameLength = AVAudioFrameCount(input[0].count)
    for ch in 0..<2 {
        for i in input[ch].indices { sourceChannels[ch][i] = input[ch][i] }
    }
    try engine.enableManualRenderingMode(.offline, format: format,
                                         maximumFrameCount: AVAudioFrameCount(quantum))
    try engine.start()
    defer { player.stop(); engine.stop(); engine.disableManualRenderingMode() }
    player.scheduleBuffer(source, completionHandler: nil)
    player.play()
    var result: [[Float]] = [[], []]
    var blocks: [Int] = []
    var chance = 16
    while result[0].count < input[0].count {
        let n = min(quantum, input[0].count - result[0].count)
        let at = engine.manualRenderingSampleTime
        destination.frameLength = 0
        switch try engine.renderOffline(AVAudioFrameCount(n), to: destination) {
        case .success:
            let actual = Int(destination.frameLength)
            try require(actual == n && engine.manualRenderingSampleTime - at == AVAudioFramePosition(actual),
                        "offline sample clock or block length did not match")
            guard let channels = destination.floatChannelData else { throw Failed.check("missing audio buffer") }
            for ch in 0..<2 {
                result[ch].append(contentsOf: UnsafeBufferPointer(start: channels[ch], count: actual))
            }
            blocks.append(actual)
            chance = 16
        case .cannotDoInCurrentContext:
            chance -= 1
            try require(chance > 0 && engine.manualRenderingSampleTime == at,
                        "offline render retries exhausted or clock advanced")
        case .insufficientDataFromInputNode: throw Failed.check("offline input unavailable")
        case .error: throw Failed.check("offline AVAudioEngine render failed")
        @unknown default: throw Failed.check("unhandled offline render status")
        }
    }
    return Capture(y: result, blocks: blocks)
}
func readHipass(_ effect: AVAudioUnitEffect) throws -> [String: Double] {
    var hz: AudioUnitParameterValue = 0, resonance: AudioUnitParameterValue = 0
    let err = AudioUnitGetParameter(effect.audioUnit, AudioUnitParameterID(kHipassParam_CutoffFrequency),
                                   kAudioUnitScope_Global, 0, &hz)
    let err2 = AudioUnitGetParameter(effect.audioUnit, AudioUnitParameterID(kHipassParam_Resonance),
                                    kAudioUnitScope_Global, 0, &resonance)
    try require(err == noErr && err2 == noErr, "HiPass readback returned \(err)/\(err2)")
    return ["cutoff_hz": Double(hz), "resonance_db": Double(resonance)]
}
func firstSignal(_ x: [Float]) -> Int { x.firstIndex { abs($0) > 0.000_000_01 } ?? -1 }

func capture_master(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let mode=repeatArm ? String(arm.dropLast(7)) : arm
 let warmFrames=16384
 let options=["sub-only":["neonverbEnable","neonunison","neonpulseVolume","neonsawVolume"],"main-only":["neonverbEnable","neonunison","neonsubVolume"]]
 guard let mutes=options[mode] else {throw Failed.usage}
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 let coldEvents=cfg["events"] as! [[String:Any]]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-warm-brainwave-master-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="BRAINWAVE MONO BASS"
 let identity="1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Bonus/DMT CYMATICS.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Bonus record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Bonus lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 let expectedActive:[String:Float] = ["neonverbEnable":1, "neonpulseVolume":1, "neonsawVolume":1, "neonsubVolume":0.726973295211792, "neonunison":1]
 for id in mutes {
   guard let k=au.parameterTree?.allParameters.first(where:{$0.identifier==id}), expectedActive[id] == k.value else {throw Failed.check("expected enabled "+id)}
   k.setValue(0,originator:nil)
 }
 try require(wait(6,{mutes.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.identifier==id})?.value==0}}),"mute not reached")
 _=wait(0.25,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let active=params()
 let pre=Dictionary(uniqueKeysWithValues:preMute.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let now=Dictionary(uniqueKeysWithValues:active.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(Set(pre.keys.filter{pre[$0] != now[$0]}) == Set(mutes+(factor==1 ? [] : [master.identifier])),"unintended parameter drift")
 guard let wanted=pinned.first(where:{($0["program"] as? String)?.caseInsensitiveCompare(p.name) == .orderedSame}) else {throw Failed.check("missing named S4 identity")}
 let fs=au.fullStateForDocument ?? [:]
 print("STATE_KEYS",fs.keys.sorted(),fs.map{($0.key,String(reflecting:type(of:$0.value)))})
 var observed:[String:String]=[:]
 for k in ["appStatePreset","lastLoadedPreset"] {
   guard let v=fs[k] as? [String:Any] else {throw Failed.check("missing "+k)}
   print("STATE_DETAIL",k,v.keys.sorted())
   for q in ["name","uid","originalUid"] { if let x=v[q] as? String {observed[k+"."+q]=x} }
   for q in ["position","version","seqTempo"] { if let x=v[q] as? NSNumber {observed[k+"."+q]=x.stringValue} }
   print("STATE_VALUES",observed)
 }
 if let selected=fs["selectedBankName"] as? String { observed["selectedBankName"]=selected }
 let expectedUid=wanted["program_uid"] as! String
 let states = ["appStatePreset","lastLoadedPreset"].map {name in
   let dict=fs[name] as! [String:Any]
   return (dict["parameterValues"] as? [[String:Any]]) ?? []
 }
 observed["appStatePreset.parameterCount"]=String(states[0].count)
 observed["lastLoadedPreset.parameterCount"]=String(states[1].count)
 let a0=states[0].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let a1=states[1].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let notEqual=a0.keys.filter{a1[$0] != nil && a1[$0] != a0[$0]}.sorted()
 let live=active.reduce(into:[String:Double]()){ $0[$1["identifier"] as! String] = $1["value"] as! Double }
 let avDiff=a1.keys.filter{live[$0] != nil && abs(a1[$0]!-live[$0]!)>1e-6}.sorted()
 observed["active_vs_lastLoaded.parameterOverlap"]=String(Set(live.keys).intersection(a1.keys).count)
 observed["active_vs_lastLoaded.parametersDiff"]=avDiff.map { "\($0):\(a1[$0]!)→\(live[$0]!)" }.joined(separator:"; ")
 print("ACTIVE_PARAM_VERIFY",observed["active_vs_lastLoaded.parameterOverlap"]!,observed["active_vs_lastLoaded.parametersDiff"]!)
 try require(Set(live.keys).intersection(a1.keys).count>100 && avDiff.count<8,"active parameters do not reflect authentic Bonus record")
 print("ACTIVE_DIFF",notEqual.map{[$0,a0[$0]! as Any,a1[$0]! as Any]})
 observed["appState_live_shared_value_differences"]=String(notEqual.count)
 try require(observed["lastLoadedPreset.uid"]?.caseInsensitiveCompare(expectedUid) == .orderedSame,"live UID differs from S4")
 if let data=fs["appState"] as? Data {
   print("APPSTATE_BYTES",data.count,"SHA",digest(data))
   observed["appState_sha256"]=digest(data)
   if let js=(try? JSONSerialization.jsonObject(with:data)) as? [String:Any] {
     print("APPSTATE_KEYS",js.keys.sorted())
     for k in ["lastLoadedPreset","lastLoaded","lastLoadedPresetUid","presetName","presetUID","uid"] {
       if let v=js[k] as? String {observed[k]=v}
       if let v=js[k] as? [String:Any] { for sub in ["name","uid"] {if let w=v[sub] as? String {observed[k+"."+sub]=w}} }
     }
   }
 }
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 while y[0].count<frames {
  let pos=y[0].count
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(peak.allSatisfy{$0>0.000001},"silent")
 let pcm=pcmBytes(y)
 let after=params()
 let post=Dictionary(uniqueKeysWithValues:after.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"muted_parameter_ids":mutes,"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
func capture_fanin(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let mode=repeatArm ? String(arm.dropLast(7)) : arm
 let warmFrames=16384
 let common=["neonverbEnable","neonunison","neoncrushAmount"]
 let options=[
   "all":common,
   "osc1-only":common+["neonsawVolume","neonsubVolume"],
   "osc2-only":common+["neonpulseVolume","neonsubVolume"],
   "sub-only":common+["neonpulseVolume","neonsawVolume"]
 ]
 guard let mutes=options[mode] else {throw Failed.usage}
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 let coldEvents=cfg["events"] as! [[String:Any]]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-warm-brainwave-fanin-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="BRAINWAVE MONO BASS"
 let identity="1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Bonus/DMT CYMATICS.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Bonus record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Bonus lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 let expectedActive:[String:Float] = ["neonverbEnable":1, "neonpulseVolume":1, "neonsawVolume":1, "neonsubVolume":0.726973295211792, "neonunison":1,"neoncrushAmount":0.11941517889499664]
 for id in mutes {
   guard let k=au.parameterTree?.allParameters.first(where:{$0.identifier==id}), expectedActive[id] == k.value else {throw Failed.check("expected enabled "+id)}
   k.setValue(0,originator:nil)
 }
 try require(wait(6,{mutes.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.identifier==id})?.value==0}}),"mute not reached")
 _=wait(0.25,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let active=params()
 let pre=Dictionary(uniqueKeysWithValues:preMute.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let now=Dictionary(uniqueKeysWithValues:active.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(Set(pre.keys.filter{pre[$0] != now[$0]}) == Set(mutes+(factor==1 ? [] : [master.identifier])),"unintended parameter drift")
 guard let wanted=pinned.first(where:{($0["program"] as? String)?.caseInsensitiveCompare(p.name) == .orderedSame}) else {throw Failed.check("missing named S4 identity")}
 let fs=au.fullStateForDocument ?? [:]
 print("STATE_KEYS",fs.keys.sorted(),fs.map{($0.key,String(reflecting:type(of:$0.value)))})
 var observed:[String:String]=[:]
 for k in ["appStatePreset","lastLoadedPreset"] {
   guard let v=fs[k] as? [String:Any] else {throw Failed.check("missing "+k)}
   print("STATE_DETAIL",k,v.keys.sorted())
   for q in ["name","uid","originalUid"] { if let x=v[q] as? String {observed[k+"."+q]=x} }
   for q in ["position","version","seqTempo"] { if let x=v[q] as? NSNumber {observed[k+"."+q]=x.stringValue} }
   print("STATE_VALUES",observed)
 }
 if let selected=fs["selectedBankName"] as? String { observed["selectedBankName"]=selected }
 let expectedUid=wanted["program_uid"] as! String
 let states = ["appStatePreset","lastLoadedPreset"].map {name in
   let dict=fs[name] as! [String:Any]
   return (dict["parameterValues"] as? [[String:Any]]) ?? []
 }
 observed["appStatePreset.parameterCount"]=String(states[0].count)
 observed["lastLoadedPreset.parameterCount"]=String(states[1].count)
 let a0=states[0].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let a1=states[1].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let notEqual=a0.keys.filter{a1[$0] != nil && a1[$0] != a0[$0]}.sorted()
 let live=active.reduce(into:[String:Double]()){ $0[$1["identifier"] as! String] = $1["value"] as! Double }
 let avDiff=a1.keys.filter{live[$0] != nil && abs(a1[$0]!-live[$0]!)>1e-6}.sorted()
 observed["active_vs_lastLoaded.parameterOverlap"]=String(Set(live.keys).intersection(a1.keys).count)
 observed["active_vs_lastLoaded.parametersDiff"]=avDiff.map { "\($0):\(a1[$0]!)→\(live[$0]!)" }.joined(separator:"; ")
 print("ACTIVE_PARAM_VERIFY",observed["active_vs_lastLoaded.parameterOverlap"]!,observed["active_vs_lastLoaded.parametersDiff"]!)
 try require(Set(live.keys).intersection(a1.keys).count>100 && avDiff.count<10,"active parameters do not reflect authentic Bonus record")
 print("ACTIVE_DIFF",notEqual.map{[$0,a0[$0]! as Any,a1[$0]! as Any]})
 observed["appState_live_shared_value_differences"]=String(notEqual.count)
 try require(observed["lastLoadedPreset.uid"]?.caseInsensitiveCompare(expectedUid) == .orderedSame,"live UID differs from S4")
 if let data=fs["appState"] as? Data {
   print("APPSTATE_BYTES",data.count,"SHA",digest(data))
   observed["appState_sha256"]=digest(data)
   if let js=(try? JSONSerialization.jsonObject(with:data)) as? [String:Any] {
     print("APPSTATE_KEYS",js.keys.sorted())
     for k in ["lastLoadedPreset","lastLoaded","lastLoadedPresetUid","presetName","presetUID","uid"] {
       if let v=js[k] as? String {observed[k]=v}
       if let v=js[k] as? [String:Any] { for sub in ["name","uid"] {if let w=v[sub] as? String {observed[k+"."+sub]=w}} }
     }
   }
 }
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 while y[0].count<frames {
  let pos=y[0].count
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(peak.allSatisfy{$0>0.000001},"silent")
 let pcm=pcmBytes(y)
 let after=params()
 let post=Dictionary(uniqueKeysWithValues:after.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"muted_parameter_ids":mutes,"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
func capture_static(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let mode=repeatArm ? String(arm.dropLast(7)) : arm
 let warmFrames=16384
 let common=["neonverbEnable","neonunison","neoncrushAmount"]
 let options=[
   "all":common,
   "osc1-only":common+["neonsawVolume","neonsubVolume"],
   "osc2-only":common+["neonpulseVolume","neonsubVolume"],
   "sub-only":common+["neonpulseVolume","neonsawVolume"]
 ]
 guard let mutes=options[mode] else {throw Failed.usage}
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 let coldEvents=cfg["events"] as! [[String:Any]]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-warm-brainwave-osc2-static-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="BRAINWAVE MONO BASS"
 let identity="1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Bonus/DMT CYMATICS.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Bonus record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Bonus lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 let expectedActive:[String:Float] = ["neonverbEnable":1, "neonpulseVolume":1, "neonsawVolume":1, "neonsubVolume":0.726973295211792, "neonunison":1,"neoncrushAmount":0.11941517889499664]
 for id in mutes {
   guard let k=au.parameterTree?.allParameters.first(where:{$0.identifier==id}), expectedActive[id] == k.value else {throw Failed.check("expected enabled "+id)}
   k.setValue(0,originator:nil)
 }
 try require(wait(6,{mutes.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.identifier==id})?.value==0}}),"mute not reached")
 _=wait(0.25,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let stageWrites:[(AUParameterAddress,Float)] = [
   (87,0),(95,0),(99,0), // no animated filter/oscillator modulation
   (129,1),(131,0),(136,0),(137,0) // full cutoff, zero track, contour and velocity
 ]
 for (id,value) in stageWrites {
   guard let p=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing static control "+String(id))}
   p.setValue(value,originator:nil)
 }
 try require(wait(6,{stageWrites.allSatisfy{id,value in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==value}}),"static writes not reached")
 _=wait(0.35,{false})
 let active=params()
 let pre=Dictionary(uniqueKeysWithValues:preMute.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let now=Dictionary(uniqueKeysWithValues:active.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let stageIDs=Set(stageWrites.map{String($0.0)})
 let stageNames=Set(active.filter{stageIDs.contains($0["address"] as? String ?? "")}.compactMap{$0["identifier"] as? String})
 try require(stageNames.count==stageWrites.count,"static write map")
 try require(Set(pre.keys.filter{pre[$0] != now[$0]}).isSubset(of:Set(mutes).union(stageNames)),"unintended parameter drift")
 for id in mutes {try require(now[id]==0,"mute changed")}
 for (id,value) in stageWrites {
   let node=active.first(where: {($0["address"] as? String)==String(id)})
   try require((node?["value"] as? Double)==Double(value),"static control drift")
 }
 guard let wanted=pinned.first(where:{($0["program"] as? String)?.caseInsensitiveCompare(p.name) == .orderedSame}) else {throw Failed.check("missing named S4 identity")}
 let fs=au.fullStateForDocument ?? [:]
 print("STATE_KEYS",fs.keys.sorted(),fs.map{($0.key,String(reflecting:type(of:$0.value)))})
 var observed:[String:String]=[:]
 for k in ["appStatePreset","lastLoadedPreset"] {
   guard let v=fs[k] as? [String:Any] else {throw Failed.check("missing "+k)}
   print("STATE_DETAIL",k,v.keys.sorted())
   for q in ["name","uid","originalUid"] { if let x=v[q] as? String {observed[k+"."+q]=x} }
   for q in ["position","version","seqTempo"] { if let x=v[q] as? NSNumber {observed[k+"."+q]=x.stringValue} }
   print("STATE_VALUES",observed)
 }
 if let selected=fs["selectedBankName"] as? String { observed["selectedBankName"]=selected }
 let expectedUid=wanted["program_uid"] as! String
 let states = ["appStatePreset","lastLoadedPreset"].map {name in
   let dict=fs[name] as! [String:Any]
   return (dict["parameterValues"] as? [[String:Any]]) ?? []
 }
 observed["appStatePreset.parameterCount"]=String(states[0].count)
 observed["lastLoadedPreset.parameterCount"]=String(states[1].count)
 let a0=states[0].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let a1=states[1].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let notEqual=a0.keys.filter{a1[$0] != nil && a1[$0] != a0[$0]}.sorted()
 let live=active.reduce(into:[String:Double]()){ $0[$1["identifier"] as! String] = $1["value"] as! Double }
 let avDiff=a1.keys.filter{live[$0] != nil && abs(a1[$0]!-live[$0]!)>1e-6}.sorted()
 observed["active_vs_lastLoaded.parameterOverlap"]=String(Set(live.keys).intersection(a1.keys).count)
 observed["active_vs_lastLoaded.parametersDiff"]=avDiff.map { "\($0):\(a1[$0]!)→\(live[$0]!)" }.joined(separator:"; ")
 print("ACTIVE_PARAM_VERIFY",observed["active_vs_lastLoaded.parameterOverlap"]!,observed["active_vs_lastLoaded.parametersDiff"]!)
 try require(Set(live.keys).intersection(a1.keys).count>100 && avDiff.filter{!stageNames.contains($0)}.count<10,"active parameters do not reflect authentic Bonus record")
 print("ACTIVE_DIFF",notEqual.map{[$0,a0[$0]! as Any,a1[$0]! as Any]})
 observed["appState_live_shared_value_differences"]=String(notEqual.count)
 try require(observed["lastLoadedPreset.uid"]?.caseInsensitiveCompare(expectedUid) == .orderedSame,"live UID differs from S4")
 if let data=fs["appState"] as? Data {
   print("APPSTATE_BYTES",data.count,"SHA",digest(data))
   observed["appState_sha256"]=digest(data)
   if let js=(try? JSONSerialization.jsonObject(with:data)) as? [String:Any] {
     print("APPSTATE_KEYS",js.keys.sorted())
     for k in ["lastLoadedPreset","lastLoaded","lastLoadedPresetUid","presetName","presetUID","uid"] {
       if let v=js[k] as? String {observed[k]=v}
       if let v=js[k] as? [String:Any] { for sub in ["name","uid"] {if let w=v[sub] as? String {observed[k+"."+sub]=w}} }
     }
   }
 }
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 while y[0].count<frames {
  let pos=y[0].count
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(peak.allSatisfy{$0>0.000001},"silent")
 let pcm=pcmBytes(y)
 let after=params()
 let post=Dictionary(uniqueKeysWithValues:after.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "osc2_static_writes":Dictionary(uniqueKeysWithValues:stageWrites.map{(String($0.0),Double($0.1))}),"ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"muted_parameter_ids":mutes,"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
func capture_physical(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let variant=repeatArm ? String(arm.dropLast(7)) : arm
 let mode="osc2-only"
 guard ["physical-authored","physical-open"].contains(variant) else {throw Failed.usage}
 let warmFrames=16384
 let common=["neonverbEnable","neonunison","neoncrushAmount"]
 let options=[
   "all":common,
   "osc1-only":common+["neonsawVolume","neonsubVolume"],
   "osc2-only":common+["neonpulseVolume","neonsubVolume"],
   "sub-only":common+["neonpulseVolume","neonsawVolume"]
 ]
 guard let mutes=options[mode] else {throw Failed.usage}
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 let coldEvents=cfg["events"] as! [[String:Any]]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-warm-brainwave-osc2-physical-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="BRAINWAVE MONO BASS"
 let identity="1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Bonus/DMT CYMATICS.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Bonus record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Bonus lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 let expectedActive:[String:Float] = ["neonverbEnable":1, "neonpulseVolume":1, "neonsawVolume":1, "neonsubVolume":0.726973295211792, "neonunison":1,"neoncrushAmount":0.11941517889499664]
 for id in mutes {
   guard let k=au.parameterTree?.allParameters.first(where:{$0.identifier==id}), expectedActive[id] == k.value else {throw Failed.check("expected enabled "+id)}
   k.setValue(0,originator:nil)
 }
 try require(wait(6,{mutes.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.identifier==id})?.value==0}}),"mute not reached")
 _=wait(0.25,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let authoredPhysical = Float(0.3345831036567688)
 guard let physical=au.parameterTree?.allParameters.first(where:{$0.address==7}),physical.identifier=="neoncutoffLayer1",physical.value==authoredPhysical else {throw Failed.check("original physical cutoff readback did not match")}
 let commonStageWrites:[(AUParameterAddress,Float)] = [
   (87,0),(95,0),(99,0), // no animated filter/oscillator modulation
   (129,1),(131,0),(136,0),(137,0) // full cutoff, zero track, contour and velocity
 ]
 let stageWrites = commonStageWrites + [(7,variant=="physical-open" ? Float(1) : authoredPhysical)]
 for (id,value) in stageWrites {
   guard let p=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing static control "+String(id))}
   p.setValue(value,originator:nil)
 }
 try require(wait(6,{stageWrites.allSatisfy{id,value in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==value}}),"static writes not reached")
 _=wait(0.35,{false})
 let active=params()
 let pre=Dictionary(uniqueKeysWithValues:preMute.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let now=Dictionary(uniqueKeysWithValues:active.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let stageIDs=Set(stageWrites.map{String($0.0)})
 let stageNames=Set(active.filter{stageIDs.contains($0["address"] as? String ?? "")}.compactMap{$0["identifier"] as? String})
 try require(stageNames.count==stageWrites.count,"static write map")
 let changed = Set(pre.keys.filter{pre[$0] != now[$0]})
 let extras = changed.subtracting(Set(mutes).union(stageNames))
 try require(extras.isEmpty,"unintended parameter drift: "+extras.sorted().joined(separator:", "))
 for id in mutes {try require(now[id]==0,"mute changed")}
 for (id,value) in stageWrites {
   let node=active.first(where: {($0["address"] as? String)==String(id)})
   try require((node?["value"] as? Double)==Double(value),"static control drift")
 }
 guard let wanted=pinned.first(where:{($0["program"] as? String)?.caseInsensitiveCompare(p.name) == .orderedSame}) else {throw Failed.check("missing named S4 identity")}
 let fs=au.fullStateForDocument ?? [:]
 print("STATE_KEYS",fs.keys.sorted(),fs.map{($0.key,String(reflecting:type(of:$0.value)))})
 var observed:[String:String]=[:]
 for k in ["appStatePreset","lastLoadedPreset"] {
   guard let v=fs[k] as? [String:Any] else {throw Failed.check("missing "+k)}
   print("STATE_DETAIL",k,v.keys.sorted())
   for q in ["name","uid","originalUid"] { if let x=v[q] as? String {observed[k+"."+q]=x} }
   for q in ["position","version","seqTempo"] { if let x=v[q] as? NSNumber {observed[k+"."+q]=x.stringValue} }
   print("STATE_VALUES",observed)
 }
 if let selected=fs["selectedBankName"] as? String { observed["selectedBankName"]=selected }
 let expectedUid=wanted["program_uid"] as! String
 let states = ["appStatePreset","lastLoadedPreset"].map {name in
   let dict=fs[name] as! [String:Any]
   return (dict["parameterValues"] as? [[String:Any]]) ?? []
 }
 observed["appStatePreset.parameterCount"]=String(states[0].count)
 observed["lastLoadedPreset.parameterCount"]=String(states[1].count)
 let a0=states[0].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let a1=states[1].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let notEqual=a0.keys.filter{a1[$0] != nil && a1[$0] != a0[$0]}.sorted()
 let live=active.reduce(into:[String:Double]()){ $0[$1["identifier"] as! String] = $1["value"] as! Double }
 let avDiff=a1.keys.filter{live[$0] != nil && abs(a1[$0]!-live[$0]!)>1e-6}.sorted()
 observed["active_vs_lastLoaded.parameterOverlap"]=String(Set(live.keys).intersection(a1.keys).count)
 observed["active_vs_lastLoaded.parametersDiff"]=avDiff.map { "\($0):\(a1[$0]!)→\(live[$0]!)" }.joined(separator:"; ")
 print("ACTIVE_PARAM_VERIFY",observed["active_vs_lastLoaded.parameterOverlap"]!,observed["active_vs_lastLoaded.parametersDiff"]!)
 try require(Set(live.keys).intersection(a1.keys).count>100 && avDiff.filter{!stageNames.contains($0)}.count<10,"active parameters do not reflect authentic Bonus record")
 print("ACTIVE_DIFF",notEqual.map{[$0,a0[$0]! as Any,a1[$0]! as Any]})
 observed["appState_live_shared_value_differences"]=String(notEqual.count)
 try require(observed["lastLoadedPreset.uid"]?.caseInsensitiveCompare(expectedUid) == .orderedSame,"live UID differs from S4")
 if let data=fs["appState"] as? Data {
   print("APPSTATE_BYTES",data.count,"SHA",digest(data))
   observed["appState_sha256"]=digest(data)
   if let js=(try? JSONSerialization.jsonObject(with:data)) as? [String:Any] {
     print("APPSTATE_KEYS",js.keys.sorted())
     for k in ["lastLoadedPreset","lastLoaded","lastLoadedPresetUid","presetName","presetUID","uid"] {
       if let v=js[k] as? String {observed[k]=v}
       if let v=js[k] as? [String:Any] { for sub in ["name","uid"] {if let w=v[sub] as? String {observed[k+"."+sub]=w}} }
     }
   }
 }
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 while y[0].count<frames {
  let pos=y[0].count
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(peak.allSatisfy{$0>0.000001},"silent")
 let pcm=pcmBytes(y)
 let after=params()
 let post=Dictionary(uniqueKeysWithValues:after.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "physical_variant":variant,"osc2_static_writes":Dictionary(uniqueKeysWithValues:stageWrites.map{(String($0.0),Double($0.1))}),"ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"muted_parameter_ids":mutes,"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
func capture_dynamic(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let variant=repeatArm ? String(arm.dropLast(7)) : arm
 let mode="osc2-only"
 guard ["physical-authored","physical-open"].contains(variant) else {throw Failed.usage}
 let warmFrames=16384
 let common=["neonverbEnable","neonunison","neoncrushAmount"]
 let options=[
   "all":common,
   "osc1-only":common+["neonsawVolume","neonsubVolume"],
   "osc2-only":common+["neonpulseVolume","neonsubVolume"],
   "sub-only":common+["neonpulseVolume","neonsawVolume"]
 ]
 guard let mutes=options[mode] else {throw Failed.usage}
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 let coldEvents=cfg["events"] as! [[String:Any]]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-warm-brainwave-osc2-live-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="BRAINWAVE MONO BASS"
 let identity="1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Bonus/DMT CYMATICS.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Bonus record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Bonus lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 let expectedActive:[String:Float] = ["neonverbEnable":1, "neonpulseVolume":1, "neonsawVolume":1, "neonsubVolume":0.726973295211792, "neonunison":1,"neoncrushAmount":0.11941517889499664]
 for id in mutes {
   guard let k=au.parameterTree?.allParameters.first(where:{$0.identifier==id}), expectedActive[id] == k.value else {throw Failed.check("expected enabled "+id)}
   k.setValue(0,originator:nil)
 }
 try require(wait(6,{mutes.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.identifier==id})?.value==0}}),"mute not reached")
 _=wait(0.25,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.3150142729282379).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let authoredPhysical = Float(0.3345831036567688)
 guard let physical=au.parameterTree?.allParameters.first(where:{$0.address==7}),physical.identifier=="neoncutoffLayer1",physical.value==authoredPhysical else {throw Failed.check("original physical cutoff readback did not match")}
 let commonStageWrites:[(AUParameterAddress,Float)] = [
   (87,0),(95,0),(99,0), // no animated filter/oscillator modulation
   (129,1),(131,0),(136,0),(137,0) // full cutoff, zero track, contour and velocity
 ]
 let stageWrites = commonStageWrites + [(7,authoredPhysical)]
 for (id,value) in stageWrites {
   guard let p=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing static control "+String(id))}
   p.setValue(value,originator:nil)
 }
 try require(wait(6,{stageWrites.allSatisfy{id,value in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==value}}),"static writes not reached")
 _=wait(0.35,{false})
 let active=params()
 let pre=Dictionary(uniqueKeysWithValues:preMute.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let now=Dictionary(uniqueKeysWithValues:active.map{($0["identifier"] as! String,$0["value"] as! Double)})
 let stageIDs=Set(stageWrites.map{String($0.0)})
 let stageNames=Set(active.filter{stageIDs.contains($0["address"] as? String ?? "")}.compactMap{$0["identifier"] as? String})
 try require(stageNames.count==stageWrites.count,"static write map")
 let changed = Set(pre.keys.filter{pre[$0] != now[$0]})
 let extras = changed.subtracting(Set(mutes).union(stageNames))
 try require(extras.isEmpty,"unintended parameter drift: "+extras.sorted().joined(separator:", "))
 for id in mutes {try require(now[id]==0,"mute changed")}
 for (id,value) in stageWrites {
   let node=active.first(where: {($0["address"] as? String)==String(id)})
   try require((node?["value"] as? Double)==Double(value),"static control drift")
 }
 guard let wanted=pinned.first(where:{($0["program"] as? String)?.caseInsensitiveCompare(p.name) == .orderedSame}) else {throw Failed.check("missing named S4 identity")}
 let fs=au.fullStateForDocument ?? [:]
 print("STATE_KEYS",fs.keys.sorted(),fs.map{($0.key,String(reflecting:type(of:$0.value)))})
 var observed:[String:String]=[:]
 for k in ["appStatePreset","lastLoadedPreset"] {
   guard let v=fs[k] as? [String:Any] else {throw Failed.check("missing "+k)}
   print("STATE_DETAIL",k,v.keys.sorted())
   for q in ["name","uid","originalUid"] { if let x=v[q] as? String {observed[k+"."+q]=x} }
   for q in ["position","version","seqTempo"] { if let x=v[q] as? NSNumber {observed[k+"."+q]=x.stringValue} }
   print("STATE_VALUES",observed)
 }
 if let selected=fs["selectedBankName"] as? String { observed["selectedBankName"]=selected }
 let expectedUid=wanted["program_uid"] as! String
 let states = ["appStatePreset","lastLoadedPreset"].map {name in
   let dict=fs[name] as! [String:Any]
   return (dict["parameterValues"] as? [[String:Any]]) ?? []
 }
 observed["appStatePreset.parameterCount"]=String(states[0].count)
 observed["lastLoadedPreset.parameterCount"]=String(states[1].count)
 let a0=states[0].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let a1=states[1].reduce(into: [String:Double]()) {if let k=$1["identifier"] as? String, let v=$1["value"] as? NSNumber {$0[k]=v.doubleValue}}
 let notEqual=a0.keys.filter{a1[$0] != nil && a1[$0] != a0[$0]}.sorted()
 let live=active.reduce(into:[String:Double]()){ $0[$1["identifier"] as! String] = $1["value"] as! Double }
 let avDiff=a1.keys.filter{live[$0] != nil && abs(a1[$0]!-live[$0]!)>1e-6}.sorted()
 observed["active_vs_lastLoaded.parameterOverlap"]=String(Set(live.keys).intersection(a1.keys).count)
 observed["active_vs_lastLoaded.parametersDiff"]=avDiff.map { "\($0):\(a1[$0]!)→\(live[$0]!)" }.joined(separator:"; ")
 print("ACTIVE_PARAM_VERIFY",observed["active_vs_lastLoaded.parameterOverlap"]!,observed["active_vs_lastLoaded.parametersDiff"]!)
 try require(Set(live.keys).intersection(a1.keys).count>100 && avDiff.filter{!stageNames.contains($0)}.count<10,"active parameters do not reflect authentic Bonus record")
 print("ACTIVE_DIFF",notEqual.map{[$0,a0[$0]! as Any,a1[$0]! as Any]})
 observed["appState_live_shared_value_differences"]=String(notEqual.count)
 try require(observed["lastLoadedPreset.uid"]?.caseInsensitiveCompare(expectedUid) == .orderedSame,"live UID differs from S4")
 if let data=fs["appState"] as? Data {
   print("APPSTATE_BYTES",data.count,"SHA",digest(data))
   observed["appState_sha256"]=digest(data)
   if let js=(try? JSONSerialization.jsonObject(with:data)) as? [String:Any] {
     print("APPSTATE_KEYS",js.keys.sorted())
     for k in ["lastLoadedPreset","lastLoaded","lastLoadedPresetUid","presetName","presetUID","uid"] {
       if let v=js[k] as? String {observed[k]=v}
       if let v=js[k] as? [String:Any] { for sub in ["name","uid"] {if let w=v[sub] as? String {observed[k+"."+sub]=w}} }
     }
   }
 }
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 var physicalSteps:[[String:Any]]=[]
 let steps:[CutoffStep]=plan ?? [CutoffStep(frame:24576,value:1),CutoffStep(frame:61440,value:Float(0.3345831036567688))]
 var step=0
 while y[0].count<frames {
  let pos=y[0].count
  if step<steps.count && pos==steps[step].frame {
    let value:Float = variant=="physical-open" ? steps[step].value : authoredPhysical
    let before=params()
    physical.setValue(value,originator:nil)
    try require(wait(6,{physical.value.bitPattern==value.bitPattern}),"physical write did not reach its readback")
    // No audio sample clock advances. Let the same observer interval elapse
    // in both the no-op and changed arm before checking the complete map.
    _=wait(0.15,{false})
    try require(engine.manualRenderingSampleTime==AVAudioFramePosition(pos),"control write advanced render clock")
    let observed=params()
    let preMap=Dictionary(uniqueKeysWithValues:before.map{($0["address"] as! String,$0["value"] as! Double)})
    let observedMap=Dictionary(uniqueKeysWithValues:observed.map{($0["address"] as! String,$0["value"] as! Double)})
    let changed=Set(preMap.keys.filter{preMap[$0] != observedMap[$0]})
    try require(changed.isSubset(of:Set(["7"])) && observedMap["7"]==Double(value),"unexpected control change")
    physicalSteps.append(["frame":pos,"raw":Double(value),"previous":preMap["7"]!,"changed":changed.sorted(),"full_readback":observed])
    step+=1
  }
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]; note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum,
            step<steps.count ? steps[step].frame-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(peak.allSatisfy{$0>0.000001},"silent")
 let pcm=pcmBytes(y)
 let after=params()
 let post=Dictionary(uniqueKeysWithValues:after.map{($0["identifier"] as! String,$0["value"] as! Double)})
 try require(step==steps.count && post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "physical_variant":variant,"physical_steps":physicalSteps,"osc2_static_writes":Dictionary(uniqueKeysWithValues:stageWrites.map{(String($0.0),Double($0.1))}),"ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"muted_parameter_ids":mutes,"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}

func capture_gain(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let mode=repeatArm ? String(arm.dropLast(7)) : arm
 let warmFrames=16384
 let polyModes=["poly-held","poly-additions","poly-all"]
 let poly=polyModes.contains(mode)
 let shapeKey=mode.components(separatedBy:"-").dropFirst().first ?? ""
 let shapeDetents:[String:Float]=["tri":Float(1.0/3.0),"saw":Float(2.0/3.0),"pulse":1]
 let shape=shapeDetents[String(shapeKey)]
 let bandMap=mode.hasPrefix("band-")
 let bandFields=mode.split(separator:"-")
 let bandNote=bandMap && bandFields.count==3 ? Int(bandFields[2]):nil
 try require(!bandMap || (bandNote != nil && (0...127).contains(bandNote!)), "invalid band cue")
 let harmonics=mode.hasPrefix("harmonic-")
 let hFields=mode.split(separator:"-")
 let hCutoffs:[String:Float]=["open":1,"c450":0.45,"c500":0.5,"c625":0.625,"c750":0.75]
 let hCutoff=harmonics && hFields.count==3 ? hCutoffs[String(hFields[2])] : nil
 try require(!harmonics || hCutoff != nil,"invalid cutoff cue")
 let compressor=mode.hasPrefix("compressor-")
 let compFields=mode.split(separator:"-").filter{!shapeDetents.keys.contains(String($0))}
 let compValue:Float=compressor && compFields.last=="on" ? 0.02 : 0
 let compFaders:[String:Float]=["f0625":0.0625,"f125":0.125,"f250":0.25,"f375":0.375,"f500":0.5,"f750":0.75,"f1000":1]
 let compFader=compressor && compFields.count==3 ? compFaders[String(compFields[1])] : nil
 try require(!compressor || (compFader != nil && (compFields.last=="on" || compFields.last=="off")),"invalid compressor arm")
 let crusher=mode.hasPrefix("crusher-active-")
 let crusherFields=mode.split(separator:"-")
 let crusherFader=crusherFields.count == 4 ? compFaders[String(crusherFields[2])] : nil
 let crusherWet=crusherFields.last=="wet"
 try require(!crusher || (crusherFader != nil && (crusherWet || crusherFields.last=="dry")), "invalid crusher cue")
 let matrix=mode.hasPrefix("crusher-matrix-")
 let fields=mode.split(separator:"-")
 let validMatrix=fields.count==5 && ["disabled","on"].contains(fields[2]) && ["a0","a1"].contains(fields[3]) && ["m0","m5","m1"].contains(fields[4])
 try require(!matrix || validMatrix,"invalid matrix cue")
 let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode) || poly || compressor || harmonics || bandMap || crusher || matrix
 let volumeSweep:[String:Float]=["response-f125":0.125,"response-f250":0.25,"response-f500":0.5,"response-f1000":1.0]
 let velocitySweep:[String:Int]=["response-v32":32,"response-v64":64,"response-v87":87,"response-v127":127]
 try require(ordinary || volumeSweep[mode] != nil || velocitySweep[mode] != nil,"unknown gain calibration arm")
 let swept = !ordinary
 let fader=((harmonics || bandMap) ? Float(1):nil) ?? compFader ?? crusherFader ?? (matrix ? Float(0.5):nil) ?? volumeSweep[mode] ?? ((mode=="osc2-only" || poly || velocitySweep[mode] != nil) ? Float(1):Float(0))
 let midiVelocity=velocitySweep[mode] ?? 87
 let fixedVelocity=velocitySweep[mode] == nil
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 // Same single A4 and signed MIDI velocity in all arms. Never move/align notes.
 let soloEvents:[[String:Any]] = [
   ["frame":16384,"note":bandNote ?? 69,"velocity":midiVelocity,"action":"on"],
   ["frame":60484,"note":bandNote ?? 69,"velocity":0,"action":"off"]
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
 } : soloEvents
 let url=workRoot.appendingPathComponent("native-jupiter-sine-gain-\(masterMode)-\(arm)",isDirectory:true)
 try require(!FileManager.default.fileExists(atPath:url.path),"destination exists")
 func wait(_ seconds: Double, _ predicate: ()->Bool) -> Bool {
  let stop=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<stop { RunLoop.current.run(until: Date().addingTimeInterval(0.020)) }
  return predicate()
 }
 let au=av.auAudioUnit
 try require(au.componentVersion==67072,"wrong version")
 let expected="Jupiter Sine"
 let identity="5A84A4E4-B9C5-41C3-853E-BB4D43D48850"
 let fileURL=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks/Intro Bank.superj6.bank.json")
 let bank=try JSONSerialization.jsonObject(with:Data(contentsOf:fileURL)) as! [String:Any]
 guard let record=(bank["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==identity}),record["name"] as? String==expected else {throw Failed.check("exact Intro Bank record missing")}
 let p=AUAudioUnitPreset()
 p.number=target
 p.name=expected
 let engine=AVAudioEngine(); let rate=Double(cfg["sample_rate"] as! Int); let frames=(cfg["capture_frames"] as! Int)+warmFrames; let quantum=cfg["block_frames"] as! Int
 guard let format = AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
       let buffer = AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(quantum)) else {throw Failed.check("buffers")}
 engine.attach(av); engine.connect(av,to:engine.mainMixerNode,format:format); engine.mainMixerNode.outputVolume=1
 try engine.enableManualRenderingMode(.offline,format:format,maximumFrameCount:AVAudioFrameCount(quantum))
 try engine.start()
 defer { engine.stop(); engine.disableManualRenderingMode() }
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record
 doc["appStatePreset"]=record
 // Keep all host metadata unchanged except this private transient preset;
 // never select/import a user library or write a saved AU state to app storage.
 au.fullStateForDocument=doc
 try require(wait(12,{
  let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
  return x?["uid"] as? String==identity && x?["name"] as? String==expected
 }),"Intro Bank lastLoaded never reached")
 _=wait(0.5,{false})
 func params()->[[String:Any]] {
  (au.parameterTree?.allParameters ?? []).map { ["identifier":$0.identifier,"display":$0.displayName,"address":String($0.address),"value":Double($0.value)] }
 }
 let preMute=params()
 func byAddress(_ incoming:[[String:Any]]) -> [String:Double] {
    Dictionary(uniqueKeysWithValues:incoming.map{($0["address"] as! String,$0["value"] as! Double)})
 }
 let baseline=byAddress(preMute)
 try require(baseline["1"]==1 && baseline["3"]==0 && baseline["82"]==0 && baseline["83"]==0,
             "authored oscillator configuration differs")
 var neutral:[AUParameterAddress:Float] = [1:mode=="osc1-only" ? 1:0,3:fader,
    7:1,8:0,11:0,16:0,17:0,22:0,23:0,24:0,28:0,32:poly ? 0:1,33:0,40:0,41:0,59:0,
    82:0,83:0,87:0,91:0,95:0,99:0,103:0,107:0,108:0,109:0,129:1,130:0,131:0,136:0,137:0,201:0,202:0,205:fixedVelocity ? 1:0,209:0]
 if compressor {neutral[17]=compValue; neutral[123]=0; if let shape {neutral[83]=shape}}
 if harmonics, let hCutoff { neutral[17]=0;neutral[123]=0;neutral[7]=hCutoff;neutral[83]=shape ?? 0 }
 if bandMap {neutral[17]=0;neutral[123]=0;neutral[7]=1;neutral[83]=shape ?? 0}
 if crusher {neutral[107]=1;neutral[108]=crusherWet ? 1:0;neutral[109]=1;neutral[123]=0}
 if matrix {neutral[107]=fields[2]=="on" ? 1:0;neutral[108]=fields[4]=="m1" ? 1:fields[4]=="m5" ? 0.5:0;neutral[109]=fields[3]=="a1" ? 1:0;neutral[123]=0}
 for (id,value) in neutral.sorted(by:{$0.key<$1.key}) {
    guard let control=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing public control "+String(id))}
    if (crusher || matrix) && id==108 && (!matrix || fields[2]=="on") {
        control.setValue(value==1 ? 0:1,originator:nil)
        _=wait(0.08,{false})
        control.setValue(value,originator:nil)
    } else if control.value != value {control.setValue(value,originator:nil)}
 }
 try require(wait(6,{neutral.allSatisfy{id,v in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==v}}),"calibration controls not reached")
 _=wait(0.35,{false})
 guard let master=au.parameterTree?.allParameters.first(where:{$0.address==0}),master.value.bitPattern==Float(0.44618064).bitPattern else {throw Failed.check("authored master readback differs")}
 let authoredMaster=master.value
 let setMaster=authoredMaster*factor
 if factor != 1 {
  master.setValue(setMaster,originator:nil)
  try require(wait(6,{master.value.bitPattern==setMaster.bitPattern}),"master write not reached")
  _=wait(0.25,{false})
 }
 let active=params()
 let now=byAddress(active)
 let intended=Set(neutral.keys.map{String($0)}).union(factor==1 ? [] : ["0"])
 let edits=Set(baseline.keys.filter{baseline[$0] != now[$0]})
 try require(edits.isSubset(of:intended),"unintended edit: "+edits.subtracting(intended).sorted().joined(separator:","))
 try require(neutral.allSatisfy{id,v in now[String(id)]==Double(v)},"live controls drift")
 let fs=au.fullStateForDocument ?? [:]
 guard let original=fs["lastLoadedPreset"] as? [String:Any],original["uid"] as? String==identity,original["name"] as? String==expected else {throw Failed.check("live preset identity changed")}
 let wanted:[String:Any]=["program":expected,"program_uid":identity,"bank":"Intro Bank"]
 let observed:[String:String]=["lastLoadedPreset.uid":identity,"lastLoadedPreset.name":expected]
 print("BEGIN",p.number,p.name,active.count); fflush(stdout)
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no public MIDI scheduler")}
 func note(_ bytes:[UInt8]) {bytes.withUnsafeBufferPointer{ midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!) }}
 var y:[[Float]]=[[],[]]
 var blocks:[Int]=[]; var retries=0; var ei=0
 while y[0].count<frames {
  let pos=y[0].count
  while ei < ev.count && ev[ei]["frame"] as! Int == pos {
    let row=ev[ei]
    if row["send"] as? Bool != false {
        note([row["action"] as! String == "on" ? 0x90 : 0x80,UInt8(row["note"] as! Int),UInt8(row["velocity"] as! Int)])
    }
    ei += 1
  }
  let n=min(quantum,frames-pos,ei<ev.count ? (ev[ei]["frame"] as! Int)-pos : quantum)
  switch try engine.renderOffline(AVAudioFrameCount(n),to:buffer) {
  case .success:
   try require(Int(buffer.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"render clock")
   guard let pcm=buffer.floatChannelData else {throw Failed.check("pcm missing")}
   for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:pcm[c],count:n))}
   blocks.append(n);retries=0
  case .cannotDoInCurrentContext:
   retries += 1; try require(retries < 24,"render retries")
  default: throw Failed.check("render failed")
  }
 }
 try require(y.allSatisfy{$0.allSatisfy(\.isFinite)},"nonfinite PCM")
 try require(y.allSatisfy{$0.prefix(warmFrames).allSatisfy{$0==0}},"unexpected pre-note signal")
 try require(blocks.prefix(128).allSatisfy{$0==128},"unexpected warm partition")
 let peak=y.map{$0.map{abs($0)}.max() ?? 0};try require(mode=="both-muted" || peak.allSatisfy{$0>0.000001},"unexpected silence")
 let pcm=pcmBytes(y)
 let after=params()
 let post=byAddress(after)
 try require(post==now,"post-render parameter drift")
 let m:[String:Any]=["reference":"official native Mac Synth One J6 only; distinct from iOS oracle","component":"aumu/j6ct/AuPr","component_version":au.componentVersion,
  "bundle_id":"com.audiokitpro.superj6.superj6AU","version":"0.999.8","build":9,"installed_au_sha256":"1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257","os":ProcessInfo.processInfo.operatingSystemVersionString,
  "ablation_arm":mode,"repeat":repeatArm,"master_mode":masterMode,"master_factor":Double(factor),"authored_master":Double(authoredMaster),"actual_master":Double(setMaster),"warm_frames":warmFrames,"fixed_velocity":fixedVelocity,"command_velocity":midiVelocity,"osc2_fader":Double(fader),"forced_controls":Dictionary(uniqueKeysWithValues:neutral.map{(String($0.key),Double($0.value))}),"pre_mute_parameters":preMute,"preset_name":p.name,"preset_number":p.number,"preset_name_after":au.currentPreset?.name ?? "-","preset_number_after":au.currentPreset?.number ?? -1,"parameters":active,"parameter_readback_after":after,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"calibration_cue":bandMap ? "jupiter-band-map-v1" : harmonics ? "jupiter-harmonic-cutoff-A4-v1" : compressor ? "jupiter-compressor-A4-v1" : (poly ? "jupiter-poly-held-v1" : (swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1")),"fixture_config_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}


// Bare detector receives the already-rendered compressor-off shape, normalized
// only for this calibration. Inlet is a conditional estimate, not an AU tap.
func compressorCalibration(_ dir:URL, _ shapeSource:[[Float]]?=nil) throws {
    let desc = AudioComponentDescription(componentType:0x61756678,componentSubType:0x64636d70,componentManufacturer:0x6170706c,componentFlags:0,componentFlagsMask:0)
    var mutableDesc=desc
    guard let component=AudioComponentFindNext(nil,&mutableDesc) else {throw Failed.check("Apple Dynamics Processor not registered")}
    var version:UInt32=0
    try require(AudioComponentGetVersion(component,&version)==0,"component version missing")
    let unit=AVAudioUnitEffect(audioComponentDescription:desc)
    let raw:Float=0.02
    let params:[UInt32:Float]=[0:Float(-20 + 30*Double(raw)),1:0.1,4:0.001,5:0.05,6:1+7*raw]
    func read() throws -> [String:Double] {
        var arr:[String:Double]=[:]
        for i:UInt32 in 0...6 {
            var p:Float=0
            try require(AudioUnitGetParameter(unit.audioUnit,i,kAudioUnitScope_Global,0,&p)==0,"Apple param readback \(i)")
            arr[String(i)]=Double(p)
        }
        return arr
    }
    let rate=44100.0, quantum=128, on=16384, off=60484, total=148684
    var results:[[String:Any]]=[]
    let meters=[0.003,0.005,0.008,0.011,0.015,0.02,0.028,0.04,0.055,0.07,0.085,0.1,0.12,0.14,0.16,0.18,0.21,0.25,0.3,0.35,0.4,0.46,0.53,0.6,0.69,0.79,0.9,0.1,0.18,0.53,0.18]
    for (index,level) in meters.enumerated() {
        let engine=AVAudioEngine(), node=AVAudioPlayerNode()
        guard let fmt=AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
            let source=AVAudioPCMBuffer(pcmFormat:fmt,frameCapacity:AVAudioFrameCount(total)),
            let dest=AVAudioPCMBuffer(pcmFormat:fmt,frameCapacity:AVAudioFrameCount(quantum)),
            let src=source.floatChannelData else {throw Failed.check("bare buffers unavailable")}
        source.frameLength=AVAudioFrameCount(total)
        for j in 0..<total {
            let t=Double(j-on)/rate
            let q:Float=(j>=on && j<off) ? Float(level*min(1,t/0.002)*sin(2*Double.pi*440*t)) : 0
            if let shapeSource {
                src[0][j]=Float(level*Double(shapeSource[0][j]))
                src[1][j]=Float(level*Double(shapeSource[1][j]))
            } else { src[0][j]=q; src[1][j]=q }
        }
        engine.attach(node); engine.attach(unit)
        engine.connect(node,to:unit,format:fmt)
        engine.connect(unit,to:engine.mainMixerNode,format:fmt)
        engine.mainMixerNode.outputVolume=1
        try engine.enableManualRenderingMode(.offline,format:fmt,maximumFrameCount:AVAudioFrameCount(quantum))
        try engine.start()
        defer {node.stop(); engine.stop();engine.disableManualRenderingMode();engine.detach(node);engine.detach(unit)}
        let isBypass=index==meters.count-1
        unit.bypass=isBypass
        for (id,p) in params { try require(AudioUnitSetParameter(unit.audioUnit,id,kAudioUnitScope_Global,0,p,0)==0,"Apple parameter set \(id)") }
        try require(AudioUnitReset(unit.audioUnit,kAudioUnitScope_Global,0)==0,"Apple unit reset")
        let before=try read()
        try require(params.allSatisfy{k,v in before[String(k)]==Double(v)},"bare readback mismatch")
        node.scheduleBuffer(source,completionHandler:nil);node.play()
        var y:[[Float]]=[[],[]]; var blocks:[Int]=[]; var retries=0
        while y[0].count<total {
            let pos=y[0].count
            let clock=engine.manualRenderingSampleTime
            let n=min(quantum,total-pos,((pos<on) ? on-pos : pos<off ? off-pos : quantum))
            switch try engine.renderOffline(AVAudioFrameCount(n),to:dest) {
            case .success:
                try require(Int(dest.frameLength)==n && clock==AVAudioFramePosition(pos) &&
                            engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"bare sample clock")
                guard let ch=dest.floatChannelData else {throw Failed.check("bare PCM absent")}
                for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:ch[c],count:n))}
                blocks.append(n);retries=0
            case .cannotDoInCurrentContext:
                retries+=1;try require(retries<24 && engine.manualRenderingSampleTime==clock,"bare retries")
            default:throw Failed.check("bare render unsuccessful")
            }
        }
        let after=try read()
        try require(after==before && y.allSatisfy{$0.allSatisfy(\.isFinite)},"bare unreadable/nonfinite")
        let out=dir.appendingPathComponent("bare-\(index).npy")
        let pcm=pcmBytes(y)
        try npy(pcm,frames:total).write(to:out,options:.atomic)
        results.append(["id":index,"peak_in":level,"bypass":isBypass,"params":before,"pcm_sha256":digest(pcm),"buffers":blocks])
    }
    let payload:[String:Any]=["input_shape":shapeSource != nil ? "normalized native compressor-off stereo PCM" : "sine", "component":"aufx/dcmp/appl","version":version,"os":ProcessInfo.processInfo.operatingSystemVersionString,
                             "rate":44100,"on":on,"off":off,"frames":total,"arms":results]
    try JSONSerialization.data(withJSONObject:payload,options:[.prettyPrinted,.sortedKeys]).write(to:dir.appendingPathComponent("bare.json"),options:.atomic)
}


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

struct CutoffStep: Codable {
    let frame: Int
    let value: Float
}
struct BenchJob: Codable {
    let id: String
    let profile: String
    let cutoffSteps: [CutoffStep]?
}
typealias ProfileCapture = (String, String, AVAudioUnit, URL, [CutoffStep]?) throws -> Void
struct BenchProfile {
    let run: ProfileCapture
    let sequence: [(String,String)]
}
let authoredCutoff = Float(0.3345831036567688)
let physicalSequence = ["physical-authored","physical-open","physical-authored-repeat","physical-open-repeat"].map{($0,"authored")}
let compressorSequence: [(String,String)] = {
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
let compressorWaveSequence:[(String,String)] = {
    var arms:[(String,String)]=[]
    for k in ["tri","saw","pulse"] {
        for f in ["f125","f250","f375","f500","f750","f1000"] {
            for state in ["off","on"] {arms.append(("compressor-"+k+"-"+f+"-"+state,"authored"))}
        }
        for state in ["on","off"] {arms.append(("compressor-"+k+"-f500-"+state+"-repeat","authored"))}
    }
    return arms
}()
let harmonicCutSequence:[(String,String)]={
    var r:[(String,String)]=[]
    for shape in ["sine","tri"] {
        for name in ["open","c450","c500","c625","c750","c500-repeat","open-repeat"] {
            r.append(("harmonic-"+shape+"-"+name,"authored"))
        }
    }
    return r
}()
let bandMapSequence:[(String,String)]={
    var rows:[(String,String)]=[]
    for shape in ["sine","tri"] {
        for key in ["57","69","81","93","105","117","45","64","76","88","100","112","69-repeat"] {
            rows.append(("band-"+shape+"-"+key,"authored"))
        }
    }
    return rows
}()
func crusherCalibration(_ dir:URL) throws {
try FileManager.default.createDirectory(at:dir, withIntermediateDirectories:true)
let desc = AudioComponentDescription(componentType:0x61756678,componentSubType:0x64697374,componentManufacturer:0x6170706c,componentFlags:0,componentFlagsMask:0)
var lookup=desc
guard let component=AudioComponentFindNext(nil,&lookup) else { throw Failed.check("Apple Distortion not registered in app") }
var version:UInt32=0
try require(AudioComponentGetVersion(component,&version)==0,"version")
let rate=44100.0, on=16384, off=60484, frames=69632
func params(_ unit:AVAudioUnitEffect) throws -> [String:Double] {
    var a:[String:Double]=[:]
    for id:UInt32 in 0...15 {
        var f:Float=0
        try require(AudioUnitGetParameter(unit.audioUnit,id,kAudioUnitScope_Global,0,&f)==0,"cannot read \(id)")
        a[String(id)]=Double(f)
    }
    return a
}
var records:[[String:Any]]=[]
for shape in ["sine"] {
    let base:[Float]=(0..<frames).map { i in
        if i<on || i>=off { return 0 }
        let t=Double(i-on)/rate
        let x=2*Double.pi*440*t
        let shapeValue = shape=="sine" ? sin(x) : (sin(x)+0.35*sin(3*x+0.4)+0.15*sin(7*x+0.9))/1.5
        return Float(min(1,t/0.002)*shapeValue)
    }
    let maxValue=base.map{abs($0)}.max()!
    for (i,level) in [0.1,0.25,0.5,0.8].enumerated() {
        for amount:Float in [0,6.06] {
        for (mode,mix) in [("m0",Float(0)),("m5",Float(35.355339)),("m1",Float(100))] {
            let unit=AVAudioUnitEffect(audioComponentDescription:desc)
            let factory=try params(unit)
            let controls:[UInt32:Float]=[2:0,3:amount,4:0,9:0,13:0,15:mix]
            for (id,p) in controls {
                try require(AudioUnitSetParameter(unit.audioUnit,id,kAudioUnitScope_Global,0,p,0)==0,"cannot set \(id)")
            }
            let before=try params(unit)
            try require(controls.allSatisfy{k,v in abs(before[String(k)]!-Double(v))<1e-5} &&
                        before["5"]==factory["5"] && before["14"]==factory["14"],"unexpected profile readback")
            let signal=base.map{$0/maxValue*Float(level)}
            let r=try render([signal,signal],rate:rate,effect:unit)
            let after=try params(unit)
            try require(after==before && r.y.allSatisfy{$0.allSatisfy(\.isFinite)},"changed profile/nonfinite")
            let f="\(shape)-\(i)-a\(amount)-\(mode).npy"
            let pcm=pcmBytes(r.y)
            try npy(pcm,frames:frames).write(to:dir.appendingPathComponent(f),options:.atomic)
            records.append(["file":f,"shape":shape,"policy":mode,"peak_in":level,"input_index":i,"amount":Double(amount),"factory":factory,
                            "before":before,"after":after,"blocks":r.blocks,"pcm_sha256":digest(pcm)])
        }
    }
}
}
let payload:[String:Any]=["component":"aufx/dist/appl","version":version,"rate":rate,"on":on,"off":off,"arms":records]
try JSONSerialization.data(withJSONObject:payload,options:[.prettyPrinted,.sortedKeys]).write(to:dir.appendingPathComponent("bare.json"),options:.atomic)

}
let profiles: [String:BenchProfile] = [
    "crusher-matrix":BenchProfile(run:capture_gain,sequence:[
        ("crusher-matrix-disabled-a0-m0","authored"),
        ("crusher-matrix-on-a0-m0","authored"),("crusher-matrix-on-a0-m5","authored"),("crusher-matrix-on-a0-m1","authored"),
        ("crusher-matrix-on-a1-m0","authored"),("crusher-matrix-on-a1-m5","authored"),("crusher-matrix-on-a1-m1","authored"),
        ("crusher-matrix-on-a1-m0-repeat","authored"),("crusher-matrix-on-a1-m1-repeat","authored")]),
    "crusher-installed":BenchProfile(run:capture_gain,sequence:[
        ("crusher-active-f250-dry","authored"),("crusher-active-f250-wet","authored"),
        ("crusher-active-f500-dry","authored"),("crusher-active-f500-wet","authored"),
        ("crusher-active-f1000-dry","authored"),("crusher-active-f1000-wet","authored"),
        ("crusher-active-f500-dry-repeat","authored"),("crusher-active-f500-wet-repeat","authored")]),
    "crusher-preflight":BenchProfile(run:capture_gain,sequence:[("both-muted","authored")]),
    "jupiter-band-map":BenchProfile(run:capture_gain,sequence:bandMapSequence),
    "jupiter-harmonic-cutoff":BenchProfile(run:capture_gain,sequence:harmonicCutSequence),
    "jupiter-compressor-waves":BenchProfile(run:capture_gain,sequence:compressorWaveSequence),
    "master": BenchProfile(run:capture_master,sequence:[("main-only","authored"),("main-only","half"),("sub-only","authored"),("sub-only","half")]),
    "fanin": BenchProfile(run:capture_fanin,sequence:["all","osc1-only","osc2-only","sub-only","all-repeat","osc1-only-repeat","osc2-only-repeat","sub-only-repeat"].map{($0,"authored")}),
    "static": BenchProfile(run:capture_static,sequence:["osc2-only","osc2-only-repeat"].map{($0,"authored")}),
    "physical": BenchProfile(run:capture_physical,sequence:physicalSequence),
    "dynamic": BenchProfile(run:capture_dynamic,sequence:physicalSequence),
    "jupiter-gain": BenchProfile(run:capture_gain,sequence:[
        ("osc1-only","authored"),("osc1-only","half"),("osc2-only","authored"),("osc2-only","half"),
        ("osc1-only-repeat","authored"),("osc2-only-repeat","authored"),("both-muted","authored")]),
    "jupiter-compressor": BenchProfile(run:capture_gain,sequence:compressorSequence),
    "jupiter-poly": BenchProfile(run:capture_gain,sequence:[
        ("poly-held","authored"),("poly-additions","authored"),("poly-all","authored"),
        ("poly-held-repeat","authored"),("poly-all-repeat","authored")]),
    "jupiter-response": BenchProfile(run:capture_gain,sequence:[
        ("response-f125","authored"),("response-f250","authored"),("response-f500","authored"),("response-f1000","authored"),
        ("response-v32","authored"),("response-v64","authored"),("response-v87","authored"),("response-v127","authored"),
        ("response-v64-repeat","authored"),("both-muted","authored")])
]

final class Bench: NSObject, NSApplicationDelegate, NSWindowDelegate {
    let root = Bundle.main.bundleURL.deletingLastPathComponent()
    let renderQueue = DispatchQueue(label:"org.dnbox.auv3.bench.render")
    let app = NSApplication.shared
    var timer: Timer?
    var pending=false
    var token=0
    var job: BenchJob?
    var currentDir: URL?
    var messages=[String]()
    let window = NSWindow(contentRect:NSRect(x:0,y:0,width:630,height:320),styleMask:[.titled,.closable,.miniaturizable],backing:.buffered,defer:false)
    let label=NSTextField(labelWithString:"Starting local AUv3 bench…")

    func applicationDidFinishLaunching(_ notification: Notification) {
        window.title="DnBOX AUv3 Bench"
        window.delegate=self
        label.frame=NSRect(x:16,y:55,width:598,height:245)
        label.maximumNumberOfLines=14
        window.contentView?.addSubview(label)
        let quit=NSButton(title:"Quit Bench",target:self,action:#selector(closeBench))
        quit.frame=NSRect(x:505,y:14,width:110,height:30)
        window.contentView?.addSubview(quit)
        window.center()
        window.makeKeyAndOrderFront(nil)
        publish("Ready. Waiting for a validated local test request.")
        let t=Timer(timeInterval:0.75,repeats:true){[weak self] _ in self?.poll()}
        timer=t
        RunLoop.main.add(t,forMode:.common)
    }
    @objc func closeBench() {app.terminate(nil)}
    func windowWillClose(_ notification: Notification) {app.terminate(nil)}
    func write(_ text:String,to url:URL) {try? text.write(to:url,atomically:true,encoding:.utf8)}
    func publish(_ message:String) {
        messages.append(message)
        if messages.count>40 {messages.removeFirst()}
        label.stringValue=messages.suffix(12).joined(separator:"\n")
        write(label.stringValue+"\n",to:root.appendingPathComponent("session-status.txt"))
        if let dir=currentDir {write(messages.joined(separator:"\n")+"\n",to:dir.appendingPathComponent("status.txt"))}
    }
    func reject(_ file:URL,_ message:String) {
        let failure=file.deletingPathExtension().appendingPathExtension("rejected")
        try? FileManager.default.moveItem(at:file,to:failure)
        publish("Rejected \(file.lastPathComponent): \(message)")
    }
    func poll() {
        if pending {return}
        let fm=FileManager.default, inbox=root.appendingPathComponent("requests",isDirectory:true)
        guard let urls=try? fm.contentsOfDirectory(at:inbox,includingPropertiesForKeys:[.isRegularFileKey,.isSymbolicLinkKey,.fileSizeKey],options:[.skipsHiddenFiles]) else {return}
        for f in urls.sorted(by:{$0.lastPathComponent<$1.lastPathComponent}) where f.pathExtension=="json" {
            let id=f.deletingPathExtension().lastPathComponent
            guard id.range(of:"^[A-Za-z0-9][A-Za-z0-9_-]{0,47}$",options:.regularExpression) != nil,
                  let m=try? f.resourceValues(forKeys:[.isRegularFileKey,.isSymbolicLinkKey,.fileSizeKey]),m.isRegularFile==true,m.isSymbolicLink==false,(m.fileSize ?? 65536)<16384,
                  let data=try? Data(contentsOf:f),let j=try? JSONDecoder().decode(BenchJob.self,from:data),j.id==id,let prof=profiles[j.profile],validate(j)
            else {reject(f,"invalid recipe or unsupported profile");continue}
            let dir=root.appendingPathComponent("jobs",isDirectory:true).appendingPathComponent(j.id,isDirectory:true)
            do {
                try fm.createDirectory(at:dir,withIntermediateDirectories:false,attributes:[.posixPermissions:0o700])
                try fm.moveItem(at:f,to:dir.appendingPathComponent("request.json"))
            } catch {reject(f,"job path already exists or is not writable");continue}
            pending=true
            job=j
            currentDir=dir
            messages=[]
            publish("Running \(j.id): \(j.profile), process \(ProcessInfo.processInfo.processIdentifier)")
            if j.profile=="crusher-preflight" {
                self.renderQueue.async {
                    do {
                        try crusherCalibration(dir.appendingPathComponent("native"))
                        DispatchQueue.main.async {self.complete(true,"24 bare Apple Distortion renders captured")}
                    } catch {DispatchQueue.main.async {self.complete(false,String(describing:error))}}
                }
                return
            }
            let binary=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/PlugIns/SuperJ6 AU.appex/Contents/MacOS/SuperJ6 AU")
            guard let bytes=try? Data(contentsOf:binary,options:.mappedIfSafe),
                  digest(bytes)=="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
            else {complete(false,"official AUv3 binary differs from pinned reference");return}
            if j.profile=="jupiter-compressor" {
                publish("Calibrating Apple Dynamics Processor")
                self.renderQueue.async {
                    do {
                        try compressorCalibration(dir)
                        DispatchQueue.main.async { self.publish("Apple calibration complete"); self.next(j,prof,0,dir) }
                    } catch { DispatchQueue.main.async { self.complete(false,String(describing:error)) } }
                }
            } else { next(j,prof,0,dir) }
            return
        }
    }
    func validate(_ j:BenchJob)->Bool {
        guard let s=j.cutoffSteps else {return true}
        guard j.profile=="dynamic",!s.isEmpty,s.count<=8,s.last?.value==authoredCutoff else {return false}
        var last=16383
        for step in s {
            guard step.frame>last,step.frame<82534,step.value.isFinite,step.value>=0,step.value<=1 else {return false}
            last=step.frame
        }
        return true
    }
    func complete(_ success:Bool,_ reason:String) {
        guard let dir=currentDir else {return}
        publish((success ? "PASS " : "FAILED ")+reason)
        let fields=["state":success ? "pass" : "failed","detail":reason,"pid":String(ProcessInfo.processInfo.processIdentifier)]
        if let bytes=try? JSONSerialization.data(withJSONObject:fields,options:[.prettyPrinted,.sortedKeys]) {
            try? bytes.write(to:dir.appendingPathComponent("result.json"),options:.atomic)
        }
        token += 1
        currentDir=nil
        job=nil
        pending=false
    }
    func next(_ j:BenchJob,_ prof:BenchProfile,_ i:Int,_ dir:URL,_ optionIndex:Int=0) {
        guard i<prof.sequence.count else {
            if j.profile=="jupiter-compressor-waves" {
                publish("Calibrating bare compressor on measured waveforms")
                self.renderQueue.async {
                    do {
                        try calibrateMeasuredWaveforms(dir)
                        DispatchQueue.main.async {self.complete(true,"all \(i) waves and matched bare calibrations complete")}
                    } catch {DispatchQueue.main.async {self.complete(false,String(describing:error))}}
                }
            } else {complete(true,"all \(i) native renders complete")}
            return
        }
        let (mode,level)=prof.sequence[i]
        let options:[AudioComponentInstantiationOptions] = [[.loadOutOfProcess],[]]
        guard optionIndex<options.count else {complete(false,"both normal loading modes failed");return}
        var desc=AudioComponentDescription(componentType:0x61756d75,componentSubType:0x6a366374,componentManufacturer:0x41755072,componentFlags:0,componentFlagsMask:0)
        guard AudioComponentFindNext(nil,&desc) != nil else {complete(false,"installed component not visible in this GUI session");return}
        token+=1
        let generation=token
        publish("Loading \(mode) \(level)")
        DispatchQueue.main.asyncAfter(deadline:.now()+15) {[weak self] in
            guard let self,self.token==generation else {return}
            self.next(j,prof,i,dir,optionIndex+1)
        }
        AVAudioUnit.instantiate(with:desc,options:options[optionIndex]) {[weak self] node,error in
            DispatchQueue.main.async {
                guard let self,self.token==generation else {return}
                self.token+=1
                guard let node else {self.publish("Load returned \(String(describing:error))");self.next(j,prof,i,dir,optionIndex+1);return}
                self.publish("Rendering \(mode) \(level)")
                self.renderQueue.async {
                    do {
                        try prof.run(mode,level,node,dir,j.cutoffSteps)
                        DispatchQueue.main.async {self.publish("Rendered \(mode) \(level)");self.next(j,prof,i+1,dir)}
                    } catch {
                        DispatchQueue.main.async {self.complete(false,String(describing:error))}
                    }
                }
            }
        }
    }
}
let app = NSApplication.shared
let bench=Bench()
app.setActivationPolicy(.accessory)
app.delegate=bench
app.run()
