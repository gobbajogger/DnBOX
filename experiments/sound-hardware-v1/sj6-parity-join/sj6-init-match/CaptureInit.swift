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
func captureInit(_ bankPath:String, _ cuePath:String, _ outPath:String) throws {
 let uid="8733EFD7-4000-4C31-BC52-5AD9E4B707E1"
 let b = try JSONSerialization.jsonObject(with: Data(contentsOf:URL(fileURLWithPath:bankPath))) as! [String:Any]
 guard let record=(b["presets"] as! [[String:Any]]).first(where:{$0["uid"] as? String==uid}),
       record["name"] as? String=="Your Init Preset",
       let values=record["parameterValues"] as? [[String:Any]], values.isEmpty else {throw Failed.check("exact official Init record missing")}
 let cueData=try Data(contentsOf:URL(fileURLWithPath:cuePath))
 let cfg=try JSONSerialization.jsonObject(with:cueData) as! [String:Any]
 let n=cfg["frames"] as! Int, rate=cfg["rate"] as! Int, blocks=cfg["buffers"] as! [Int]
 let events=cfg["schedule"] as! [[String:Any]]
 try require(blocks.reduce(0,+)==n,"frame sum")
 let description=AudioComponentDescription(componentType:0x61756d75,componentSubType:0x6a366374,componentManufacturer:0x41755072,componentFlags:0,componentFlagsMask:0)
 var result:AVAudioUnit?, err:Error?, finished=false
 AVAudioUnit.instantiate(with:description,options:[.loadOutOfProcess]) { a,e in result=a;err=e;finished=true }
 func wait(_ seconds:Double,_ predicate:()->Bool)->Bool{
  let end=Date().addingTimeInterval(seconds)
  while !predicate() && Date()<end {RunLoop.current.run(until:Date().addingTimeInterval(0.015))}
  return predicate()
 }
 try require(wait(12,{finished}),"AU timeout")
 guard let node=result else {throw Failed.check("AU instantiate: \(String(describing:err))")}
 let au=node.auAudioUnit
 try require(au.componentVersion==67072,"unqualified component version \(au.componentVersion)")
 let eng=AVAudioEngine()
 let format=AVAudioFormat(commonFormat:.pcmFormatFloat32,sampleRate:Double(rate),channels:2,interleaved:false)!
 let buf=AVAudioPCMBuffer(pcmFormat:format,frameCapacity:AVAudioFrameCount(blocks.max()!))!
 eng.attach(node);eng.connect(node,to:eng.mainMixerNode,format:format);eng.mainMixerNode.outputVolume=1
 try eng.enableManualRenderingMode(.offline,format:format,maximumFrameCount:buf.frameCapacity)
 try eng.start()
 defer {eng.stop();eng.disableManualRenderingMode()}
 var doc=au.fullStateForDocument ?? [:]
 doc["lastLoadedPreset"]=record;doc["appStatePreset"]=record
 au.fullStateForDocument=doc
 try require(wait(12,{(au.fullStateForDocument?["lastLoadedPreset"] as? [String:Any])?["uid"] as? String==uid}),"Init recall not identity-bound")
 _=wait(0.5,{false})
 func words()->[String:UInt32] {Dictionary(uniqueKeysWithValues:(au.parameterTree?.allParameters ?? []).map { (String($0.address),$0.value.bitPattern) })}
 let before=words()
 try require(before.count>=160,"missing Init controls")
 let labels=Dictionary(uniqueKeysWithValues:(au.parameterTree?.allParameters ?? []).map{(String($0.address),$0.identifier)})
 guard let midi=au.scheduleMIDIEventBlock else {throw Failed.check("no MIDI")}
 var y:[[Float]]=[[],[]];var pos=0;var ei=0
 for block in blocks {
  while ei<events.count && events[ei]["frame"] as! Int==pos {
   let evt=events[ei];let key=UInt8(evt["note"] as! Int), vel=UInt8(evt["velocity"] as! Int)
   let msg=[UInt8(vel==0 ? 0x80 : 0x90),key,vel]
   msg.withUnsafeBufferPointer {midi(AUEventSampleTimeImmediate,0,$0.count,$0.baseAddress!)}
   ei+=1
  }
  try require(ei==events.count || events[ei]["frame"] as! Int>=pos+block,"nonaligned note")
  let stamp=eng.manualRenderingSampleTime
  var attempts=16
  while true {
   buf.frameLength=0
   let rc=try eng.renderOffline(AVAudioFrameCount(block),to:buf)
   if rc == .success {break}
   attempts-=1
   try require(rc == .cannotDoInCurrentContext && attempts>0 && eng.manualRenderingSampleTime==stamp,"offline render error")
  }
  try require(Int(buf.frameLength)==block && eng.manualRenderingSampleTime==stamp+Int64(block),"sample clock")
  for c in 0..<2 {y[c].append(contentsOf:UnsafeBufferPointer(start:buf.floatChannelData![c],count:block))}
  pos+=block
 }
 let after=words()
 try require(before==after,"control drift")
 try require(pos==n && ei==events.count && y.allSatisfy({$0.allSatisfy(\.isFinite)}) && y[0].contains(where: {abs($0)>0.00001}),"invalid or silent render")
 let raw=pcmBytes(y)
 try FileManager.default.createDirectory(atPath:outPath,withIntermediateDirectories:false)
 let out=URL(fileURLWithPath:outPath,isDirectory:true)
 try raw.write(to:out.appendingPathComponent("audio.f32"))
 let receipt:[String:Any]=["uid":uid,"name":"Your Init Preset","component_version":au.componentVersion,"raw_controls":before,"identifiers":labels,"rate":rate,"frames":n,"events":ei,"blocks":blocks.count,"cue_sha256":digest(cueData),"signed_pcm_sha256":digest(raw),"peak":y.map{$0.map{abs($0)}.max()!}]
 try JSONSerialization.data(withJSONObject:receipt,options:[.prettyPrinted,.sortedKeys]).write(to:out.appendingPathComponent("capture.json"))
 print("Init capture complete: \(n) frames, \(before.count) stable controls, \(ei) events, SHA \(digest(raw))")
}
do {
 try require(CommandLine.arguments.count==4,"CaptureInit <official User Bank> <cue.json> <new output directory>")
 try captureInit(CommandLine.arguments[1],CommandLine.arguments[2],CommandLine.arguments[3])
} catch {fputs("\(error)\n",stderr);exit(1)}
