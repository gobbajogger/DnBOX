import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import replay
import factory
from factory import canonical_bytes
from replay import authored_layers, digest


class AuthoredPwmTests(unittest.TestCase):
    def test_first_match_presence_and_main_pass_defaults(self):
        keys = {n: str(n) for n in (78, 79, 82, 83, 210, 211)}
        parameters = [(82, 1.), (83, 1.), (78, -.0), (78, .75), (211, .0)]
        record = {"uid": "synthetic", "parameterValues": [
            {"identifier": keys[key], "value": value} for key, value in parameters]}
        first, second = list(authored_layers(record, keys))
        for row, expected in ((first, "00000080"), (second, "00000000")):
            self.assertEqual(struct.pack("<f", row[3]).hex(), expected)
            self.assertEqual(struct.pack("<f", row[4]).hex(), "00000000")
        self.assertEqual(struct.pack("<f", first[5]["authored_pwm_amount"]).hex(), "00000080")
        self.assertIsNone(first[5]["authored_pwm_enable"])
        self.assertIsNone(second[5]["authored_pwm_amount"])
        self.assertEqual(second[5]["authored_pwm_enable"], .0)


class SelectedRecordTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name)
        self.selected = root / "matrix"
        self.archive = root / "synthetic.zip"
        self.archive.write_bytes(b"synthetic archive test input")
        bank = {"uuid": "TEST-BANK"}
        records, rows = {}, []
        catalog = json.loads((replay.HERE.parent.parent / "mapping/results/implementer_brief/public_control_catalog.json").read_bytes())
        keys = {r["id"]: r["identifier"] for r in catalog["controls"]}
        for i in range(14):
            uid = f"TEST-{i:02}"
            record = {"uid": uid, "name": uid, "position": i, "parameterValues": [
                {"identifier": keys[k], "value": v} for k, v in
                ((82, 1.), (83, 1.), (78, -0.), (78, .75), (211, 1.))]}
            records[uid] = record
            rows.append({"bank_uuid": bank["uuid"], "program_uid": uid, "program": uid,
                         "record_sha256": digest(canonical_bytes(record))})
        self.banks = {bank["uuid"]: (bank, records)}
        spec = {"archive_sha256": digest(self.archive.read_bytes()), "programs": rows}
        spec_file = root / "s4-inputs.json"
        spec_file.write_bytes(canonical_bytes(spec))
        with patch.object(factory, "HERE", root):
            factory.materialize_matrix(self.banks, self.selected)
        self.files = [self.selected / f"{uid}.json" for uid in records]
        audit = {"spec_sha256": digest(spec_file.read_bytes()), "programs": [
            {"uid": path.stem, "supplied_bank_uuid": bank["uuid"],
             "file_sha256": digest(path.read_bytes())} for path in self.files]}
        audit_file = root / "audit.json"
        audit_file.write_bytes(canonical_bytes(audit))
        patched = patch.multiple(replay, SPEC=spec, SPEC_FILE=spec_file, AUDIT=audit_file)
        patched.start()
        self.addCleanup(patched.stop)

    def settings(self):
        return list(replay.cases(None, "controls", self.selected))

    def test_mixed_matrix_and_extracted_records_match_archive_settings(self):
        for path in self.files[::2]:
            record = json.loads(path.read_bytes())["preset"]
            path.write_text(json.dumps(record, indent=2))
        with patch.object(replay, "load_factory", return_value=self.banks) as load:
            archived = list(replay.cases(self.archive, "controls"))
            selected = self.settings()
            load.assert_called_once_with(self.archive)
        self.assertEqual(len(selected), 28)
        self.assertEqual(selected, archived)
        for index, row in enumerate(selected):
            self.assertEqual(row[0], self.files[index // 2].stem)
            self.assertEqual(struct.pack("<f", row[3]), struct.pack("<f", 0. if row[1] else -0.))
            self.assertEqual(row[5]["authored_pwm_amount"] is None, bool(row[1]))
            self.assertEqual(row[5]["authored_pwm_enable"] is None, not row[1])

    def test_wrapped_bytes_uid_types_and_missing_records_are_checked(self):
        path = self.files[-1]
        raw = path.read_bytes()
        item = json.loads(raw)
        preset = item["preset"]
        for bad, reason in (
            (raw + b" ", "matrix file checksum"),
            (canonical_bytes({**item, "bank": {"uuid": "ANOTHER-BANK"}}), "bank UUID"),
            (canonical_bytes({**preset, "uid": "ANOTHER-PRESET"}), "preset object with UID"),
            (raw.replace(b'"preset":', b'"preset":null,"preset":'), "duplicate JSON key"),
            (canonical_bytes({**preset, "position": float(preset["position"])}), "record checksum"),
            (canonical_bytes(preset).replace(b"-0.0", b"0.0"), "record checksum"),
            (canonical_bytes(preset).replace(b"1.0", b"true"), "record checksum"),
        ):
            with self.subTest(reason=reason):
                path.write_bytes(bad)
                with self.assertRaisesRegex(ValueError, reason):
                    self.settings()
        path.unlink()
        out = self.selected.parent / "render"
        argv = ["replay", "--compact", ".", "--repo", ".", "--selected-records", str(self.selected), "--out", str(out)]
        with patch.object(replay.sys, "argv", argv), patch.object(replay, "build") as build:
            with self.assertRaisesRegex(ValueError, "missing selected S4 record"):
                replay.main()
            build.assert_not_called()
        self.assertFalse(out.exists())


if __name__ == "__main__":
    unittest.main()
