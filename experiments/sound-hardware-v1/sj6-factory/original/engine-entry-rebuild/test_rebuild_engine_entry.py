import importlib.util
import struct
import subprocess
import sys
from pathlib import Path
import unittest

import rebuild_engine_entry as runner


def make_test_image():
    """A synthetic Mach image with a deterministic mock imported BL."""
    base = runner.INIT & ~0xfff
    mock_stub = base + 0xc00
    test = bytearray(4096)

    struct.pack_into("<8I", test, 0, 0xFEEDFACF, 0x0100000C, 0, 8, 3, 256, 0, 0)
    command = 32
    struct.pack_into("<2I16s4Q4I", test, command, 0x19, 152, b"__TEXT",
                     base, 4096, 0, 4096, 7, 5, 1, 0)
    section = command + 72
    struct.pack_into("<16s16s2Q8I", test, section, b"__stubs", b"__TEXT",
                     mock_stub, 12, 0xc00, 2, 0, 0, 8, 0, 12, 0)
    command += 152
    struct.pack_into("<6I", test, command, 2, 24, 0xff0, 1, 0xf80, 0x30)
    command += 24
    struct.pack_into("<2I", test, command, 0xb, 80)
    struct.pack_into("<2I", test, command + 56, 0xf70, 1)
    struct.pack_into("<I", test, 0xff0, 1)
    name = runner.UNSUPPORTED_IMPORT.encode()
    test[0xf81:0xf81 + len(name)] = name

    for pc in range(runner.INIT, runner.FIRST_CALL, 4):
        struct.pack_into("<I", test, pc - base, 0xd503201f)
    branch = 0x94000000 | ((mock_stub - runner.FIRST_CALL) >> 2)
    struct.pack_into("<I", test, runner.FIRST_CALL - base, branch)
    return bytes(test), mock_stub


class EngineEntryTests(unittest.TestCase):
    def test_forward_and_backward_bl(self):
        self.assertEqual(runner.arm64_bl_target(0x1000, 0x94000002), 0x1008)
        self.assertEqual(runner.arm64_bl_target(0x1000, 0x97ffffff), 0xffc)
        with self.assertRaises(ValueError):
            runner.arm64_bl_target(0x1000, 0xd503201f)

    def test_mach_import_and_rejection(self):
        data, stub = make_test_image()
        tables = runner.macho_tables(data)
        self.assertEqual(runner.import_at(data, tables, stub), runner.UNSUPPORTED_IMPORT)
        with self.assertRaises(ValueError):
            runner.import_at(data, tables, stub + 4)
        with self.assertRaises(ValueError):
            runner.macho_tables(bytes(len(data)))

    def test_chained_stub_requires_pointer_in_page_chain(self):
        data, stub = make_test_image()
        data = bytearray(data)
        struct.pack_into("<3I", data, 0xc00, 0x90000010, 0xf9461210, 0xd61f0200)
        struct.pack_into("<Q", data, 0xc18, 2 << 51)
        struct.pack_into("<Q", data, 0xc20, 1 << 63)
        struct.pack_into("<7I", data, 0xd00, 0, 28, 60, 64, 1, 1, 0)
        struct.pack_into("<2I", data, 0xd1c, 1, 8)
        struct.pack_into("<IHHQIHH", data, 0xd24, 24, 4096, 6, 0, 0, 1, 0xc18)
        struct.pack_into("<I", data, 0xf70, 0xc0000000)
        data[0xd40:0xd4b] = b"_test_bind\0"
        tables = list(runner.macho_tables(data))
        tables[4] = (0xd00, 0x80)
        self.assertEqual(runner.import_at(data, tables, stub), "_test_bind")
        struct.pack_into("<Q", data, 0xc18, 0)
        with self.assertRaisesRegex(ValueError, "not an on-chain bind"):
            runner.import_at(data, tables, stub)

    @unittest.skipUnless(importlib.util.find_spec("unicorn"), "Unicorn not available")
    def test_synthetic_exec_stops_at_import(self):
        data, stub = make_test_image()
        result = runner.execute_prefix(data)
        self.assertEqual(result["stub"], hex(stub))
        self.assertEqual(result["executed_original_instructions"], 11)
        self.assertEqual(result["status"], "BLOCKED_AT_REAL_FRAMEWORK_IMPORT")

    def test_release_bank_container_profile(self):
        banks, expected = {}, []
        next_id = 0
        for index in range(12):
            name = f"bank-{index}"
            count = 54 if index < 4 else 53
            presets = []
            for _ in range(count):
                controls = 142 if next_id < 309 else 141
                presets.append({"uid": f"preset-{next_id}", "parameterValues": [None] * controls})
                next_id += 1
            release_id = f"release-{index}"
            banks[name] = {"uuid": release_id, "presets": presets}
            expected.append({"name": name, "release_bank_uuid": release_id,
                             "export_bank_uuid": f"export-{index}", "release_programs": count})
        banks["User Bank"] = {"uuid": "release-only-bank", "presets": [{"uid": "init", "parameterValues": []}]}
        expected.append({"name": "User Bank", "release_bank_uuid": "release-only-bank",
                         "export_bank_uuid": None, "release_programs": 1})
        audit = {"ipa_sha256": runner.IPA_SHA, "au_sha256": runner.EXE_SHA,
                 "export_zip_sha256": runner.BANK_SHA, "banks": expected,
                 "unpaired_programs": [{"bank": "User Bank", "uid": "init", "present_only_in": "release"}]}
        report = runner.release_container_identity(banks, audit)
        self.assertEqual(report["factory_program_count"], 640)
        self.assertEqual(report["authored_control_words"], 90549)
        self.assertEqual(report["banks"][1]["release_uuid"], "release-0")
        self.assertEqual(report["banks"][1]["export_uuid"], "export-0")
        banks["bank-0"]["presets"][0]["parameterValues"].pop()
        with self.assertRaisesRegex(ValueError, "ordered control count"):
            runner.release_container_identity(banks, audit)

    def test_missing_input_refuses_render(self):
        command = [sys.executable, Path(runner.__file__), "--repo", "/absent-repo",
                   "--ipa-recovery", "/absent-ipa.zip"]
        result = subprocess.run(command, capture_output=True, text=True, check=False)
        self.assertEqual(result.returncode, 2)
        self.assertIn("MISSING_PINNED_INPUTS", result.stdout)
        self.assertIn('"original_stereo_captures": 0', result.stdout)


if __name__ == "__main__":
    unittest.main()
