// Exact public-control Raw solo witness. Requires existing AU bench declarations.
// In its profile table add:
// "raw-solo":BenchProfile(run:capture_factory,sequence:["dry","main-only","sub-only","main-only-repeat","sub-only-repeat","dry-repeat"].map{("2",$0)}),
// Build the same AppKit host with this factory method replacing the original;
// use local job {"id":"raw-solo-v1","profile":"raw-solo"}. No DSP-device writes.
// Status: isolated host compiled and signature verified, but LaunchServices
// returned -10827 before startup in this environment. No new PCM was captured.
// Existing signed Raw and 48 kHz Main-only tape stay the active test anchors.
func capture_factory(_ arm:String,_ mode:String,_ av:AVAudioUnit,_ root:URL,_ plan:[CutoffStep]?) throws {
    guard let ix=Int(arm), (0..<4).contains(ix), ["authored","dry","authored-repeat","main-only","sub-only","main-only-repeat","sub-only-repeat","dry-repeat"].contains(mode) else { throw Failed.usage }
    let bytes=try Data(contentsOf:Bundle.main.url(forResource:"s4-inputs",withExtension:"json")!)
    try require(digest(bytes)=="17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943","wrong cue digest")
    let cfg=try JSONSerialization.jsonObject(with:bytes) as! [String:Any]
    let wanted=(cfg["programs"] as! [[String:Any]])[ix]
    let name=wanted["program"] as! String, uid=wanted["program_uid"] as! String
    let bankRoot=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/Frameworks/SuperJ6_Assets.framework/Versions/A/Resources/PresetBanks", isDirectory:true)
    let bankName=wanted["bank"] as! String
    let file=bankRoot.appendingPathComponent((ix==0 ? "Bonus/" : "")+bankName+".superj6.bank.json")
    let loaded=try JSONSerialization.jsonObject(with:Data(contentsOf:file)) as! [String:Any]
    let source=loaded["presets"] as! [[String:Any]]
    guard let record=source.first(where:{$0["uid"] as? String==uid}),record["name"] as? String==name else {throw Failed.check("wrong factory record")}
    let au=av.auAudioUnit
    try require(au.componentVersion==67072,"wrong AU version")
    let rate=Double(cfg["sample_rate"] as! Int), q=cfg["block_frames"] as! Int, warm=16384, frames=(cfg["capture_frames"] as! Int)+16384
    let schedule=(cfg["events"] as! [[String:Any]]).map { incoming -> [String:Any] in
        var e=incoming; e["frame"]=(incoming["frame"] as! Int)+warm; return e
    }
    let engine=AVAudioEngine()
    guard let fmt=AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:rate,channels:2,interleaved:false),
          let buf=AVAudioPCMBuffer(pcmFormat:fmt,frameCapacity:AVAudioFrameCount(q)) else {throw Failed.check("cannot allocate PCM")}
    engine.attach(av);engine.connect(av,to:engine.mainMixerNode,format:fmt);engine.mainMixerNode.outputVolume=1
    try engine.enableManualRenderingMode(.offline,format:fmt,maximumFrameCount:AVAudioFrameCount(q));try engine.start()
    defer {engine.stop();engine.disableManualRenderingMode()}
    func wait(_ t:Double,_ f:()->Bool)->Bool {
        let until=Date().addingTimeInterval(t)
        while !f() && Date()<until {RunLoop.current.run(until:Date().addingTimeInterval(0.02))}
        return f()
    }
    func params()->[[String:Any]] {
        (au.parameterTree?.allParameters ?? []).map{["identifier":$0.identifier,"address":String($0.address),"value":Double($0.value)]}
    }
    func mapped(_ rows:[[String:Any]])->[String:Double] {
        rows.reduce(into:[:]) { r,x in if let k=x["identifier"] as? String,let v=x["value"] as? NSNumber {r[k]=v.doubleValue}}
    }
    var state=au.fullStateForDocument ?? [:];state["lastLoadedPreset"]=record;state["appStatePreset"]=record;au.fullStateForDocument=state
    func selected()->Bool {
        let x=au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any]
        return x?["name"] as? String==name && x?["uid"] as? String==uid
    }
    try require(wait(10,selected),"factory identity not loaded")
    _=wait(0.5,{false})
    let pre=params(), pm=mapped(pre), saved=mapped(record["parameterValues"] as? [[String:Any]] ?? [])
    let overlap=Set(pm.keys).intersection(saved.keys)
    let mismatch=overlap.sorted().filter{abs(pm[$0]!-saved[$0]!)>1e-6}
    try require(overlap.count>100 && mismatch.count<20,"too many authored differences: \(overlap.count) / \(mismatch)")
    let ids:[AUParameterAddress]=[11,16,17,22,23,24,40,41,107]
    var edits:[String]=[]
    let originalDry = mode != "authored" && mode != "authored-repeat"
    if originalDry {
        for id in ids {
            guard let p=au.parameterTree?.allParameters.first(where:{$0.address==id}) else {throw Failed.check("missing dry effect address \(id)")}
            edits.append(p.identifier)
            if id==107 && p.value==0 {p.setValue(1,originator:nil);_=wait(0.12,{false})}
            if id==17 && p.value==0 {p.setValue(0.02,originator:nil);_=wait(0.12,{false})}
            if p.value != 0 {p.setValue(0,originator:nil)}
        }
        try require(wait(6,{ids.allSatisfy{id in au.parameterTree?.allParameters.first(where:{$0.address==id})?.value==0}}),"dry FX control readback")
        _=wait(0.35,{false})
    }
    if mode.hasPrefix("main-only") || mode.hasPrefix("sub-only") {
        try require(ix==2,"solo mode requires exact J106")
        let address: AUParameterAddress = mode.hasPrefix("main-only") ? 202 : 1
        guard let p=au.parameterTree?.allParameters.first(where:{$0.address==address}) else {throw Failed.check("missing public solo mute")}
        edits.append(p.identifier)
        p.setValue(0,originator:nil)
        try require(wait(6,{p.value==0}),"isolated source mute did not apply")
        _=wait(0.35,{false})
    }
    let before=params(), bm=mapped(before)
    try require(Set(pm.keys.filter{pm[$0] != bm[$0]}).isSubset(of:Set(edits)),"non-effect control unexpectedly changed")
    guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("MIDI scheduler absent")}
    var output:[[Float]]=[[],[]], blocks:[Int]=[], next=0, retries=0
    while output[0].count<frames {
        let pos=output[0].count
        while next<schedule.count && schedule[next]["frame"] as! Int==pos {
            let e=schedule[next]
            let msg:[UInt8]=[e["action"] as! String=="on" ? 0x90:0x80,UInt8(e["note"] as! Int),UInt8(e["velocity"] as! Int)]
            msg.withUnsafeBufferPointer {midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!)}
            next+=1
        }
        let n=min(q,frames-pos,next<schedule.count ? (schedule[next]["frame"] as! Int)-pos:q)
        switch try engine.renderOffline(AVAudioFrameCount(n),to:buf) {
        case .success:
            try require(Int(buf.frameLength)==n && engine.manualRenderingSampleTime==AVAudioFramePosition(pos+n),"wrong audio clock")
            guard let pcm=buf.floatChannelData else {throw Failed.check("missing PCM")}
            for ch in 0..<2 {output[ch].append(contentsOf:UnsafeBufferPointer(start:pcm[ch],count:n))}
            retries=0;blocks.append(n)
        case .cannotDoInCurrentContext:
            retries+=1;try require(retries<24,"render retries")
        default:throw Failed.check("native render failed")
        }
    }
    let after=params()
    try require(selected() && mapped(after)==bm,"after-render state drift")
    try require(output.allSatisfy{$0.allSatisfy(\.isFinite)} && output.contains{ $0.dropFirst(warm).contains{abs($0)>1e-5} },"nonfinite or silent")
    let pcm=pcmBytes(output)
    let item=root.appendingPathComponent("program-\(ix)-\(mode)",isDirectory:true)
    try FileManager.default.createDirectory(at:item,withIntermediateDirectories:false)
    try npy(pcm,frames:frames).write(to:item.appendingPathComponent("output.npy"),options:.atomic)
    let meta:[String:Any]=["component":"aumu/j6ct/AuPr","component_version":au.componentVersion,"wanted":wanted,"mode":mode,
        "bank_file_sha256":digest(try Data(contentsOf:file)),"cue_sha256":digest(bytes),"rate":Int(rate),"frames":frames,"warm_frames":warm,
        "schedule":schedule,"blocks":blocks,"pcm_sha256":digest(pcm),"authored_overlap":overlap.count,"authored_readback_mismatch":mismatch,
        "authored":pre,"active":before,"after":after,"dry_control_identifiers":edits]
    try JSONSerialization.data(withJSONObject:meta,options:[.prettyPrinted,.sortedKeys]).write(to:item.appendingPathComponent("capture.json"),options:.atomic)
}
