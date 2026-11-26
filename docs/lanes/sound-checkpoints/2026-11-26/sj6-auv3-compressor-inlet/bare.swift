func compressorCalibration(_ dir:URL) throws {
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
            src[0][j]=q; src[1][j]=q
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
    let payload:[String:Any]=["component":"aufx/dcmp/appl","version":version,"os":ProcessInfo.processInfo.operatingSystemVersionString,
                             "rate":44100,"on":on,"off":off,"frames":total,"arms":results]
    try JSONSerialization.data(withJSONObject:payload,options:[.prettyPrinted,.sortedKeys]).write(to:dir.appendingPathComponent("bare.json"),options:.atomic)
}
