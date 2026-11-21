// Host harness, not part of original app. Original classes/math are prepended.
// No A4 retuning callback is invented; original default middle C is retained.
let arguments = CommandLine.arguments
precondition(arguments.count == 4)
let sampleRate = Double(arguments[2])!
precondition([44100.0, 48000.0, 96000.0].contains(sampleRate))
AKSettings.sampleRate = sampleRate
let input = try Data(contentsOf: URL(fileURLWithPath: arguments[1]))
let cases = try JSONSerialization.jsonObject(with: input) as! [[String: Any]]
var output = [[String: Any]]()
for scenario in cases {
    let table = AKTuningTable()
    if let master = scenario["master"] as? [Double] {
        precondition(table.tuningTable(fromFrequencies: master) == master.count)
    }
    var doubles = [UInt64]()
    var floats = [UInt32]()
    for note in 0..<128 {
        let value = table.frequency(forNoteNumber: MIDINoteNumber(note))
        precondition(value.isFinite && value > 0 && value <= sampleRate / 2)
        doubles.append(value.bitPattern)
        floats.append(Float(value).bitPattern)
    }
    output.append(["id": scenario["id"]!, "npo": table.npo,
                   "double_words": doubles, "float_words": floats])
}
let result: [String: Any] = ["sample_rate": sampleRate, "tables": output,
    "scope": "original AudioKit tuning math; not original app callbacks or audio"]
let encoded = try JSONSerialization.data(withJSONObject: result, options: [.sortedKeys])
try encoded.write(to: URL(fileURLWithPath: arguments[3]))
