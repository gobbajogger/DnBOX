import AudioToolbox
import AVFoundation
import Foundation
import XCTest
@testable import ReferenceBench

final class SuperJ6ReferenceTests: XCTestCase {
    private let resources = Bundle.module
    private func data(_ name: String) throws -> Data {
        guard let url = resources.url(forResource: name, withExtension: "json", subdirectory: "Inputs") else {
            throw BenchFailure("Missing test resource \(name).json")
        }
        return try Data(contentsOf: url)
    }

    private func attach(_ data: Data, _ name: String, _ type: String) {
        let item = XCTAttachment(data: data, uniformTypeIdentifier: type)
        item.name = name
        item.lifetime = .keepAlways
        add(item)
    }

    func testFactoryPresetCaptures() async throws {
        let source = try data("s4-inputs")
        let spec = try JSONDecoder().decode(Matrix.self, from: source)
        let audit = try JSONDecoder().decode(ReleaseAudit.self, from: data("bundled-bank-audit"))
        try require(sha(source) == audit.spec_sha256 && spec.programs.count == 14 &&
                    spec.sample_rate == 44100 && spec.channels == 2 && spec.gain == 1 &&
                    spec.block_frames == 128 && spec.capture_frames == 132300, "Unrecognized matrix")

        let registered = AVAudioUnitComponentManager.shared().components(matching: component)
        try require(registered.count == 1 && registered[0].version == Int(audit.component_version),
                    "Expected one registered aumu/AuPr/jay6 release build")
        var liveDigest: String?
        #if os(macOS)
        if let url = registered[0].componentURL, let executable = Bundle(url: url)?.executableURL {
            liveDigest = sha(try Data(contentsOf: executable))
            try require(liveDigest == audit.au_sha256, "Registered AU executable differs")
        }
        #endif

        var failures: [String] = []
        for row in spec.programs {
            do {
                let input = try data(row.program_uid)
                guard let checked = audit.programs.first(where: { $0.uid == row.program_uid }),
                      checked.supplied_bank_uuid == row.bank_uuid, checked.file_sha256 == sha(input) else {
                    throw BenchFailure("Input file failed the release audit")
                }
                let package = try dictionary(JSONSerialization.jsonObject(with: input))
                let preset = try dictionary(package["preset"])
                let unit = try await instance()
                let format = AVAudioFormat(standardFormatWithSampleRate: Double(spec.sample_rate), channels: 2)!
                try unit.outputBusses[0].setFormat(format)
                unit.maximumFramesToRender = AUAudioFrameCount(spec.block_frames)
                unit.isRenderingOffline = true
                unit.musicalContextBlock = nil
                unit.transportStateBlock = nil
                let block = unit.renderBlock
                guard let schedule = unit.scheduleMIDIEventBlock else { throw BenchFailure("Missing MIDI block") }
                try unit.allocateRenderResources()
                defer { unit.deallocateRenderResources() }
                let initialTree = Set(unit.parameterTree?.allParameters.map { $0.identifier } ?? [])
                let (route, restored) = try await loadProgram(unit, row: row, source: preset)
                let active = try dictionary(restored["appStatePreset"])
                let application = try auditApplied(unit, source: preset, active: active,
                                                   initialTree: initialTree, route: route)
                let (pcm, events, firstRender) = try render(unit.outputBusses[0].format, block, schedule, spec)
                let savedState = try JSONSerialization.data(withJSONObject: active, options: [.sortedKeys])
                var receipt: Record = [
                    "arm": "original", "bank_uuid": row.bank_uuid, "bundled_bank_uuid": checked.bundled_bank_uuid,
                    "program_uid": row.program_uid, "record_sha256": row.record_sha256,
                    "s4_inputs_sha256": audit.spec_sha256, "factory_route": route,
                    "component_version": audit.component_version, "sample_rate": spec.sample_rate,
                    "authored_tempo": preset["seqTempo"]!, "applied_tempo": active["seqTempo"]!,
                    "first_event_frame": events[0]["frame"]!, "first_render_frame": firstRender,
                    "scheduled_events": events, "block_frames": unit.maximumFramesToRender,
                    "gain": 1.0, "reset": spec.reset, "transport": spec.transport,
                    "input_file_sha256": sha(input),
                    "event_count": spec.events.count,
                    "frames": spec.capture_frames, "channels": spec.channels, "format": "stereo-f32le",
                    "pcm_sha256": sha(pcm), "applied_state_sha256": sha(savedState),
                    "complete_preset": application.complete, "applied_readback": application.details,
                    "release_au_sha256": audit.au_sha256,
                    "runtime_system": ProcessInfo.processInfo.operatingSystemVersionString
                ]
                if let digest = liveDigest { receipt["au_sha256"] = digest }
                attach(savedState, row.program_uid + ".applied.json", "public.json")
                attach(pcm, row.program_uid + ".f32le", "public.data")
                attach(try JSONSerialization.data(withJSONObject: receipt, options: [.prettyPrinted, .sortedKeys]),
                       row.program_uid + ".json", "public.json")
                if !application.complete {
                    failures.append(row.program_uid + " incomplete applied controls; see readback in attachment")
                }
            } catch {
                failures.append("\(row.program_uid) \(row.program): \(error)")
            }
        }
        XCTAssertTrue(failures.isEmpty, failures.joined(separator: "\n"))
    }
}
