func capture_sub_zones(_ arm:String,_ masterMode:String,_ av:AVAudioUnit,_ workRoot:URL,_ plan:[CutoffStep]?) throws {
 let target = -1
 let factors:[String:Float]=["authored":1,"half":0.5,"quarter":0.25]
 guard let factor=factors[masterMode] else {throw Failed.usage}
 let repeatArm=arm.hasSuffix("-repeat")
 let label=repeatArm ? String(arm.dropLast(7)) : arm
 guard let key=Int(label), (0...127).contains(key) else { throw Failed.usage }
 let mode="sub-only"
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
 let coldEvents:[[String:Any]]=[
 ["action":"on","frame":0,"note":key,"velocity":100],
 ["action":"off","frame":28672,"note":key,"velocity":0]
 ]
 let ev = coldEvents.map { incoming -> [String:Any] in
   var item=incoming
   item["frame"] = (item["frame"] as! Int) + warmFrames
   return item
 }
 let pinned = (cfg["programs"] as! [[String:Any]])
 let url=workRoot.appendingPathComponent("native-brainwave-sub-zone-\(masterMode)-\(arm)",isDirectory:true)
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
  "test":"fresh absolute signed sub-only onset and release, source Range fixed at authored lowest",
  "cue_midi":key,"source-zone-root-candidate":key-12,
  "rate":Int(rate),"frames":frames,"buffers":blocks,"schedule":ev,"s4_input_sha256":digest(file),"s4_pinned_identity":wanted,"observed_state":observed,"state_keys":fs.keys.sorted(),"peak":peak,"pcm_sha256":digest(pcm)]
 try FileManager.default.createDirectory(at:url,withIntermediateDirectories:false)
 try npy(pcm,frames:frames).write(to:url.appendingPathComponent("output.npy"),options:.atomic)
 try JSONSerialization.data(withJSONObject:m,options:[.prettyPrinted,.sortedKeys]).write(to:url.appendingPathComponent("capture.json"),options:.atomic)
 print("CAPTURE",p.name,peak,digest(pcm))
}
