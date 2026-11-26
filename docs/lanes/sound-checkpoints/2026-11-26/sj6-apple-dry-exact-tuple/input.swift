import Foundation
let rate=44100.0,on=16384,off=60484,frames=69632
let base:[Float]=(0..<frames).map { i in
    if i<on || i>=off { return 0 }
    let t=Double(i-on)/rate
    let x=2*Double.pi*440*t
    return Float(min(1,t/0.002)*sin(x))
}
let maxValue=base.map{abs($0)}.max()!
for (idx, level) in [0.025,0.05,0.1,0.15,0.2,0.3,0.4,0.5,0.65,0.8,1.0,1.2].enumerated() {
    let signal=base.map{$0/maxValue*Float(level)}
    let bytes=signal.withUnsafeBytes { Data($0) }
    try bytes.write(to:URL(fileURLWithPath:"\(idx).f32"))
}
