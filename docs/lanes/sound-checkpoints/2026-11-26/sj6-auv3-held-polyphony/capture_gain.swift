func capture_gain(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let mode=repeatArm ? String(arm.dropLast(7)) : arm
 let warmFrames=16384
 let polyModes=["poly-held","poly-additions","poly-all"]
 let poly=polyModes.contains(mode)
 let ordinary=["osc1-only","osc2-only","both-muted"].contains(mode) || poly
 let volumeSweep:[String:Float]=["response-f125":0.125,"response-f250":0.25,"response-f500":0.5,"response-f1000":1.0]
 let velocitySweep:[String:Int]=["response-v32":32,"response-v64":64,"response-v87":87,"response-v127":127]
 try require(ordinary || volumeSweep[mode] != nil || velocitySweep[mode] != nil,"unknown gain calibration arm")
 let swept = !ordinary
 let fader=volumeSweep[mode] ?? ((mode=="osc2-only" || poly || velocitySweep[mode] != nil) ? Float(1):Float(0))
 let midiVelocity=velocitySweep[mode] ?? 87
 let fixedVelocity=velocitySweep[mode] == nil
 let file=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
 try require(digest(file)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong S4 input digest")
 let cfg = try JSONSerialization.jsonObject(with:file) as! [String:Any]
 // Same single A4 and signed MIDI velocity in all arms. Never move/align notes.
 let soloEvents:[[String:Any]] = [
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
 let neutral:[AUParameterAddress:Float] = [1:mode=="osc1-only" ? 1:0,3:fader,
    7:1,8:0,11:0,16:0,17:0,22:0,23:0,24:0,28:0,32:poly ? 0:1,33:0,40:0,41:0,59:0,
    82:0,83:0,87:0,91:0,95:0,99:0,103:0,107:0,108:0,109:0,129:1,130:0,131:0,136:0,137:0,201:0,202:0,205:fixedVelocity ? 1:0,209:0]
 for (id,value) in neutral.sorted(by:{$0.key<$1.key}) {
    guard let control=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing public control "+String(id))}
    if control.value != value {control.setValue(value,originator:nil)}
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
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"calibration_cue":poly ? "jupiter-poly-held-v1" : (swept ? "jupiter-response-A4-v1":"jupiter-gain-A4-v1"),"fixture_config_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
