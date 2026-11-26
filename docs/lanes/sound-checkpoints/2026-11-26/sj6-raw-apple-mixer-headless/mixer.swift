import AVFoundation
import Foundation

let rate = 44100.0
let targetChannels: AVAudioChannelCount = 2
let target = AVAudioFormat(standardFormatWithSampleRate: rate, channels: targetChannels)!
enum MixerProbeError: Error { case badRender(AVAudioEngineManualRenderingStatus) }

func probe(_ channels: [AVAudioChannelCount], _ values: [[Float]]) throws -> [Float] {
    let engine = AVAudioEngine()
    let branch = AVAudioMixerNode()
    engine.attach(branch)
    var keep: [AVAudioSourceNode] = []
    for (i, count) in channels.enumerated() {
        let format = AVAudioFormat(standardFormatWithSampleRate: rate, channels: count)!
        let amplitude = values[i]
        let source = AVAudioSourceNode(format: format) { _, _, frames, output in
            let buffer = UnsafeMutableAudioBufferListPointer(output)
            for (index, item) in buffer.enumerated() {
                let data = item.mData!.assumingMemoryBound(to: Float.self)
                for f in 0..<Int(frames) { data[f] = amplitude[index] }
            }
            return noErr
        }
        engine.attach(source)
        engine.connect(source, to: branch, fromBus: 0, toBus: AVAudioNodeBus(i), format: format)
        keep.append(source)
    }
    engine.connect(branch, to: engine.mainMixerNode, format: target)
    try engine.enableManualRenderingMode(.offline, format: target, maximumFrameCount: 128)
    engine.prepare()
    try engine.start()
    defer { engine.stop(); engine.disableManualRenderingMode() }
    let output = AVAudioPCMBuffer(pcmFormat: target, frameCapacity: 128)!
    for _ in 0..<32 {
        let result = try engine.renderOffline(128, to: output)
        guard result == .success, output.frameLength == 128 else {
            throw MixerProbeError.badRender(result)
        }
    }
    let pcm = output.floatChannelData!
    let exact = (0..<128).allSatisfy { pcm[0][$0] == pcm[0][127] && pcm[1][$0] == pcm[1][127] }
    precondition(exact)
    return [pcm[0][127], pcm[1][127]]
}

let cases: [(String, [AVAudioChannelCount], [[Float]])] = [
    ("mono", [1], [[0.1875]]),
    ("stereo", [2], [[0.1875, -0.375]]),
    ("stereo_4_buses_only_first", [2,2,2,2], [[0.1875,-0.375],[0,0],[0,0],[0,0]]),
    ("stereo_4_buses_only_third", [2,2,2,2], [[0,0],[0,0],[0.1875,-0.375],[0,0]]),
    ("stereo_4_buses_two_active", [2,2,2,2], [[0.1875,-0.375],[0,0],[0.3125,0.125],[0,0]]),
    ("mono_4_buses_only_first", [1,1,1,1], [[0.1875],[0],[0],[0]]),
    ("mono_4_buses_two_active", [1,1,1,1], [[0.1875],[0],[0.3125],[0]]),
    ("mixed_4_buses_two_active", [2,2,1,2], [[0.1875,-0.375],[0,0],[0.3125],[0,0]])
]
var rows: [String:[Float]] = [:]
for (name, ch, y) in cases {
    rows[name] = try probe(ch, y)
}
let body: [String:Any] = ["rate": rate, "mode": "offline synthetic; no source app", "rows": rows]
let json = try JSONSerialization.data(withJSONObject: body, options: [.prettyPrinted, .sortedKeys])
print(String(decoding: json, as: UTF8.self))
