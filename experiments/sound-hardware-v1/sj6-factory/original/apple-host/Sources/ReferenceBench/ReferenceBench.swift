import AudioToolbox
import AVFoundation
import CryptoKit
import Foundation

typealias Record = [String: Any]

struct BenchFailure: Error, CustomStringConvertible {
    let description: String
    init(_ text: String) { description = text }
}

func sha(_ data: Data) -> String {
    SHA256.hash(data: data).map { String(format: "%02x", $0) }.joined()
}

func require(_ condition: Bool, _ message: String) throws {
    if !condition { throw BenchFailure(message) }
}

func dictionary(_ object: Any?) throws -> Record {
    if let result = object as? Record { return result }
    if let data = object as? Data {
        if let result = try JSONSerialization.jsonObject(with: data) as? Record { return result }
    }
    throw BenchFailure("Missing AU preset dictionary")
}

struct Event: Decodable {
    let frame: Int
    let note: UInt8
    let velocity: UInt8
    let action: String
}

struct Program: Decodable {
    let bank: String
    let bank_uuid: String
    let program: String
    let program_uid: String
    let record_sha256: String
}

struct Matrix: Decodable {
    let sample_rate: Int
    let channels: Int
    let capture_frames: Int
    let block_frames: Int
    let gain: Double
    let reset: String
    let transport: String
    let programs: [Program]
    let events: [Event]
}

struct AuditedRecord: Decodable {
    let uid: String
    let file_sha256: String
    let supplied_bank_uuid: String
    let bundled_bank_uuid: String
}

struct ReleaseAudit: Decodable {
    let component_version: UInt32
    let au_sha256: String
    let spec_sha256: String
    let programs: [AuditedRecord]
}

let component = AudioComponentDescription(componentType: kAudioUnitType_MusicDevice,
                                                  componentSubType: 0x6a617936, // jay6
                                                  componentManufacturer: 0x41755072, // AuPr
                                                  componentFlags: 0, componentFlagsMask: 0)

func instance() async throws -> AUAudioUnit {
    try await withCheckedThrowingContinuation { continuation in
        AUAudioUnit.instantiate(with: component, options: []) { unit, error in
            if let unit = unit { continuation.resume(returning: unit) }
            else { continuation.resume(throwing: error ?? BenchFailure("AU instantiation returned nil")) }
        }
    }
}

// The real build-9 AU returns appStatePreset and lastLoadedPreset via fullState.
// Check the selected program's complete authored controls in their serialized order.
// appStatePreset may include migrated/default fields. The original record is retained
// in lastLoadedPreset and must still identify the selected program.
private func authoredMatch(_ saved: Record, _ supplied: Record) -> Bool {
    guard saved["uid"] as? String == supplied["uid"] as? String,
          let a = saved["parameterValues"] as? [Record],
          let b = supplied["parameterValues"] as? [Record], a.count == b.count else { return false }
    // The release audit found two metadata differences in the export. They identify
    // bank placement and ancestry. All other authored metadata, including tempo and
    // microtuning, must survive the original loader.
    for (key, expected) in supplied where !["parameterValues", "position", "originalUid"].contains(key) {
        guard let value = saved[key] as? NSObject, value.isEqual(expected) else { return false }
    }
    return zip(a, b).allSatisfy { lhs, rhs in
        guard let x = lhs["value"] as? NSNumber, let y = rhs["value"] as? NSNumber else { return false }
        return lhs["identifier"] as? String == rhs["identifier"] as? String
            && x.floatValue.bitPattern == y.floatValue.bitPattern
    }
}

private func selectedState(_ unit: AUAudioUnit, row: Program, source: Record) -> Record? {
    guard let state = unit.fullState,
          let original = try? dictionary(state["lastLoadedPreset"]),
          let active = try? dictionary(state["appStatePreset"]),
          authoredMatch(original, source),
          (active["uid"] as? String == row.program_uid ||
           active["originalUid"] as? String == row.program_uid),
          let savedTempo = active["seqTempo"] as? NSNumber, let authoredTempo = source["seqTempo"] as? NSNumber,
          savedTempo.doubleValue == authoredTempo.doubleValue,
          let live = active["parameterValues"] as? [Record], !live.isEmpty else { return nil }
    return state
}

private func waitForSelection(_ unit: AUAudioUnit, row: Program, source: Record) async throws -> Record? {
    for _ in 0..<250 {
        if let verified = selectedState(unit, row: row, source: source) { return verified }
        try await Task.sleep(nanoseconds: 20_000_000)
    }
    return nil
}

func loadProgram(_ unit: AUAudioUnit, row: Program, source: Record) async throws -> (String, Record) {
    // The export bank number and position are unrelated to the AU preset number.
    let candidates = (unit.factoryPresets ?? []).filter { $0.name == row.program }
    for item in candidates {
        unit.currentPreset = item
        if let verified = try await waitForSelection(unit, row: row, source: source) {
            return ("factoryPresets/\(item.number)", verified)
        }
    }
    guard var state = unit.fullState else { throw BenchFailure("Original AU returned no fullState template") }
    // Keep AU metadata, use the AU's own document restore for exact exported content.
    state["selectedBankName"] = row.bank
    state["lastLoadedPreset"] = source
    state["appStatePreset"] = source
    state.removeValue(forKey: "j6LibraryAncestor")
    unit.fullState = state
    guard let verified = try await waitForSelection(unit, row: row, source: source) else {
        throw BenchFailure("AU did not confirm preset UID and authored parameter record for \(row.program)")
    }
    return ("fullState", verified)
}

func render(_ format: AVAudioFormat, _ block: AURenderBlock, _ schedule: AUScheduleMIDIEventBlock,
            _ spec: Matrix) throws -> (Data, [Record], Int) {
    guard let buffer = AVAudioPCMBuffer(pcmFormat: format,
                                        frameCapacity: AVAudioFrameCount(spec.block_frames)),
          let channels = buffer.floatChannelData else { throw BenchFailure("Missing MIDI or float stereo output") }
    var result = Data(capacity: spec.capture_frames * 8)
    var cursor = 0
    var scheduled: [Record] = []
    var firstRender: Int?
    var sounding = [false, false]
    for begin in stride(from: 0, to: spec.capture_frames, by: spec.block_frames) {
        let count = min(spec.block_frames, spec.capture_frames - begin)
        while cursor < spec.events.count && spec.events[cursor].frame < begin + count {
            let event = spec.events[cursor]
            try require(event.frame >= begin && (event.action == "on" || event.action == "off"),
                        "Invalid event order or action")
            let status: UInt8 = event.action == "on" ? 0x90 : 0x80
            [status, event.note, event.velocity].withUnsafeBufferPointer { bytes in
                schedule(AUEventSampleTime(event.frame), 0, bytes.count, bytes.baseAddress!)
            }
            scheduled.append(["frame": event.frame, "action": event.action, "note": event.note, "velocity": event.velocity])
            cursor += 1
        }
        buffer.frameLength = AVAudioFrameCount(count)
        for channel in 0..<2 { channels[channel].update(repeating: 0, count: count) }
        var flags = AudioUnitRenderActionFlags()
        var stamp = AudioTimeStamp()
        stamp.mFlags = .sampleTimeValid
        stamp.mSampleTime = Double(begin)
        let status = block(&flags, &stamp, AUAudioFrameCount(count), 0, buffer.mutableAudioBufferList, nil)
        try require(status == noErr, "AU render failed at frame \(begin) with OSStatus \(status)")
        if firstRender == nil { firstRender = Int(stamp.mSampleTime) }
        let output = UnsafeMutableAudioBufferListPointer(buffer.mutableAudioBufferList)
        try require(output.count == 2 && output.allSatisfy { $0.mData != nil && $0.mDataByteSize >= UInt32(count * 4) },
                    "AU returned an incomplete stereo buffer")
        let returned = output.map { $0.mData!.assumingMemoryBound(to: Float.self) }
        for i in 0..<count {
            for channel in 0..<2 {
                let sample = returned[channel][i]
                try require(sample.isFinite, "Non-finite original PCM at frame \(begin + i)")
                sounding[channel] = sounding[channel] || sample != 0
                var word = sample.bitPattern.littleEndian
                withUnsafeBytes(of: &word) { result.append(contentsOf: $0) }
            }
        }
    }
    try require(cursor == spec.events.count && sounding.contains(true) && firstRender != nil, "Incomplete event schedule or silent original")
    return (result, scheduled, firstRender!)
}


struct ApplicationAudit {
    let complete: Bool
    let details: Record
}

// Capture applicability from the authenticated record, before inspecting the AU's
// control tree. A small intersection, an echoed state dictionary, a new alias or a
// linked-control difference is insufficient for a qualified restore.
func auditApplied(_ unit: AUAudioUnit, source: Record, active: Record,
                  initialTree: Set<String>, route: String) throws -> ApplicationAudit {
    guard let expected = source["parameterValues"] as? [Record],
          let snapshot = active["parameterValues"] as? [Record],
          let tree = unit.parameterTree, !expected.isEmpty else {
        throw BenchFailure("Missing authored fields, active snapshot or live AU tree")
    }
    var saved: [String: Float] = [:]
    var live: [String: AUParameter] = [:]
    var seen = Set<String>()
    var missingActive: [String] = []
    var missingTree: [String] = []
    var changed: [Record] = []
    var activeChanged: [Record] = []
    var unexposedLegacy: [String] = []
    var liveWords: [String: UInt32] = [:]
    var liveMatches = 0
    for entry in snapshot {
        guard let key = entry["identifier"] as? String, let value = entry["value"] as? NSNumber,
              saved.updateValue(value.floatValue, forKey: key) == nil else {
            throw BenchFailure("Invalid or duplicate snapshot parameter")
        }
    }
    for param in tree.allParameters {
        guard live.updateValue(param, forKey: param.identifier) == nil else {
            throw BenchFailure("Duplicate AU parameter identifier " + param.identifier)
        }
    }
    for entry in expected {
        guard let key = entry["identifier"] as? String, let value = entry["value"] as? NSNumber,
              seen.insert(key).inserted else { throw BenchFailure("Invalid or duplicate authored parameter") }
        let bits = value.floatValue.bitPattern
        let legacy = key == "neonlimiterAmount" || key == "neontubeRageAmount"
        if let stateValue = saved[key] {
            if stateValue.bitPattern != bits {
                activeChanged.append(["identifier": key, "expected_bits": bits, "active_bits": stateValue.bitPattern])
            }
        } else { missingActive.append(key) }

        guard let parameter = live[key] else {
            if legacy && !initialTree.contains(key) { unexposedLegacy.append(key) }
            else { missingTree.append(key) }
            continue
        }
        let actualBits = parameter.value.bitPattern
        liveWords[key] = actualBits
        if actualBits == bits { liveMatches += 1 }
        else {
            changed.append(["identifier": key, "expected_bits": bits, "applied_bits": actualBits,
                            "min": parameter.minValue, "max": parameter.maxValue])
        }
    }
    // Neither legacy name joins a current BaseParameter in the authenticated graph.
    // On a native factory selection the original loader owns their migration. Still
    // require both to round-trip in active state. A document-state restore cannot
    // attribute their DSP effect from an echoed record, so keep it diagnostic.
    let legacyOK = unexposedLegacy.isEmpty || route.hasPrefix("factoryPresets/")
    let complete = missingActive.isEmpty && activeChanged.isEmpty && missingTree.isEmpty
        && changed.isEmpty && liveMatches > 0 && legacyOK
    let details: Record = ["authored": expected.count, "snapshot": snapshot.count,
        "tree_before_selection": initialTree.count, "tree_after_selection": live.count,
        "live_matches": liveMatches, "live_words": liveWords, "missing_active": missingActive, "active_differences": activeChanged,
        "missing_live": missingTree, "live_differences": changed, "unexposed_legacy": unexposedLegacy,
        "qualified": complete]
    return ApplicationAudit(complete: complete, details: details)
}
