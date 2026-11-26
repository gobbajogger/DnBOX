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
    let quantum = 256
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
let dir = URL(fileURLWithPath: CommandLine.arguments[1], isDirectory:true)
try FileManager.default.createDirectory(at:dir, withIntermediateDirectories:true)
let desc = AudioComponentDescription(componentType:0x61756678,componentSubType:0x64697374,componentManufacturer:0x6170706c,componentFlags:0,componentFlagsMask:0)
var lookup=desc
let component=AudioComponentFindNext(nil,&lookup)
var version:UInt32=0
if let component { try require(AudioComponentGetVersion(component,&version)==0,"version") }
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
for shape in ["sine","harmonic"] {
    let base:[Float]=(0..<frames).map { i in
        if i<on || i>=off { return 0 }
        let t=Double(i-on)/rate
        let x=2*Double.pi*440*t
        let shapeValue = shape=="sine" ? sin(x) : (sin(x)+0.35*sin(3*x+0.4)+0.15*sin(7*x+0.9))/1.5
        return Float(min(1,t/0.002)*shapeValue)
    }
    let maxValue=base.map{abs($0)}.max()!
    for (i,level) in [0.05,0.1,0.2,0.4,0.8,1.2,0.2].enumerated() {
        for (mode,mix) in [("dry",Float(0)),("wet",Float(100))] {
            let unit=AVAudioUnitEffect(audioComponentDescription:desc)
            let factory=try params(unit)
            let controls:[UInt32:Float]=[2:0,3:6.06,4:0,9:0,13:0,15:mix]
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
            let f="\(shape)-\(i)-\(mode).npy"
            let pcm=pcmBytes(r.y)
            try npy(pcm,frames:frames).write(to:dir.appendingPathComponent(f),options:.atomic)
            records.append(["file":f,"shape":shape,"policy":mode,"peak_in":level,"input_index":i,"factory":factory,
                            "before":before,"after":after,"blocks":r.blocks,"pcm_sha256":digest(pcm)])
        }
    }
}
let payload:[String:Any]=["component":"aufx/dist/appl","version":version,"rate":rate,"on":on,"off":off,"arms":records]
try JSONSerialization.data(withJSONObject:payload,options:[.prettyPrinted,.sortedKeys]).write(to:dir.appendingPathComponent("bare.json"),options:.atomic)
