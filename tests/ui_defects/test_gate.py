# SPDX-License-Identifier: GPL-3.0-only
"""Aggregator self-tests only; not actual product visual acceptance."""
import copy
import importlib.util
import hashlib
import struct
import tempfile
from pathlib import Path
import unittest
from PIL import Image

spec = importlib.util.spec_from_file_location("gate", Path(__file__).with_name("gate.py"))
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)
H = "a" * 64
_temporary = tempfile.TemporaryDirectory()
PNG = Path(_temporary.name) / "unit-screen.png"
# Deliberately synthetic valid image fixture, never a product-render receipt.
Image.new("RGB",(1,1),(0,0,0)).save(PNG)
ARTIFACT = {"path":str(PNG),"sha256":hashlib.sha256(PNG.read_bytes()).hexdigest(),
            "sourceClosureSha256":H,"kind":"synthetic-unit-fixture"}


def model():
    return {"sourceClosureSha256": H, "checks": [{"id": i, "status": "PASS", "evidence": [copy.deepcopy(ARTIFACT)]} for i in range(1, 25)],
            "canonicalSpecSHA256":"c9e69f4885a85e51862ef1092af168760f50075a1daf4222f0be2486e0c649ad",
            "screens": [{**ARTIFACT,"id": "test", "width":1,"height":1,"score": 10, "defects": []}],
            "coverageInventory":[{"id":"test"}],
            "coverageComplete": True, "run": {"exit": 0, "firstAttempt": True, "sourceClosureSha256": H}}


class GateTests(unittest.TestCase):
    def test_missing_or_wrong_canonical_spec_binding_blocks(self):
        r=model();r.pop("canonicalSpecSHA256")
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))
        r=model();r["canonicalSpecSHA256"]="b"*64
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))

    def test_all_inapplicable_screens_do_not_establish_coverage(self):
        r=model();r["screens"]=[{"id":"all","applicability":"NOT_APPLICABLE","reason":"unsupported"}]
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))

    def test_truncated_png_header_is_not_a_render(self):
        truncated=Path(_temporary.name)/"truncated.png"
        truncated.write_bytes(b"\x89PNG\r\n\x1a\n"+b"\x00\x00\x00\x0dIHDR"+struct.pack(">II",1,1))
        r=model();r["screens"][0].update(path=str(truncated),sha256=hashlib.sha256(truncated.read_bytes()).hexdigest())
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))

    def test_missing_or_incomplete_explicit_inventory_blocks(self):
        for inv in [[],[{"id":"different"}],[{"id":"test"},{"id":"test"}]]:
            r=model();r["coverageInventory"]=inv
            self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))

    def test_internal_valid_contract(self):
        self.assertFalse(gate.inspect_report(model(), H, gate.FIRMWARE_IDS, "firmware"))

    def test_each_missing_required_check_blocks(self):
        for cid in gate.FIRMWARE_IDS:
            r = model(); r["checks"] = [c for c in r["checks"] if c["id"] != cid]
            self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_each_nonpass_status_blocks(self):
        for state in ["FAIL", "REVIEW_REQUIRED", "NOT_RUN", "SKIP", None]:
            r = model(); r["checks"][0]["status"] = state
            self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_pass_without_evidence_blocks(self):
        r = model(); r["checks"][0]["evidence"] = []
        self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_stale_render_or_source_blocks(self):
        r = model(); r["screens"][0]["sourceClosureSha256"] = "b" * 64
        self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))
        self.assertTrue(gate.inspect_report(model(), "b" * 64, gate.FIRMWARE_IDS, "firmware"))

    def test_review_not_accepted_blocks(self):
        for score in [None, 0, 8, 9, 9.9]:
            r = model(); r["screens"][0]["score"] = score
            self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_human_finding_needs_real_red_green(self):
        r = model(); r["humanDefects"] = [{"id": "D01"}]
        self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))
        r["negatives"] = [{"defectId": "D01", "oldSourceClosureSha256": "b" * 64, "oldExit": 1,
                            "currentSourceClosureSha256": H, "currentExit": 0,"evidence":[copy.deepcopy(ARTIFACT)]}]
        self.assertFalse(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))
        r["negatives"][0]["oldExit"] = 0
        self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_clean_first_run_required(self):
        r = model(); r["run"]["firstAttempt"] = False
        self.assertTrue(gate.inspect_report(r, H, gate.FIRMWARE_IDS, "firmware"))

    def test_pushed_readback_required(self):
        r = model(); self.assertTrue(gate.verify_pushed_revision(r, {}, "firmware"))
        r["revision"] = "1" * 40
        self.assertFalse(gate.verify_pushed_revision(r, {"head": {"sha": "1" * 40}}, "firmware"))
        self.assertTrue(gate.verify_pushed_revision(r, {"head": {"sha": "2" * 40}}, "firmware"))

    def test_forged_evidence_hash_blocks(self):
        r = model();r["checks"][0]["evidence"] = [{"path":str(Path(__file__)),"sha256":"0"*64,
            "sourceClosureSha256":H,"kind":"synthetic-unit-fixture"}]
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))

    def test_missing_screen_artifact_blocks(self):
        r=model();r["screens"][0].update(path="does-not-exist.png",sha256="0"*64,width=240,height=240)
        self.assertTrue(gate.inspect_report(r,H,gate.FIRMWARE_IDS,"firmware"))


if __name__ == "__main__":
    unittest.main()
