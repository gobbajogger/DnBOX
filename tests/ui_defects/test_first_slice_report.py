# SPDX-License-Identifier: GPL-3.0-only
"""Synthetic adapter contract tests, not product acceptance."""
import importlib.util
from pathlib import Path
import unittest
spec=importlib.util.spec_from_file_location("adapter",Path(__file__).with_name("first_slice_report.py"))
adapter=importlib.util.module_from_spec(spec);spec.loader.exec_module(adapter)
H="a"*64
class AdapterTests(unittest.TestCase):
    def test_partial_result_cannot_become_qualification(self):
        raw={"sourceClosureSha256":H,"checks":{"1":{"status":"PASS"},"3":{"status":"FAIL","failures":[{"reason":"space"}]}},
             "screens":[],"run":{"exit":0,"firstAttempt":True},"detectorExit":1,"coverageComplete":False}
        r=adapter.normalize(raw,[],[])
        self.assertFalse(r["coverageComplete"]);self.assertFalse(r["run"]["firstAttempt"])
        self.assertEqual(r["run"]["exit"],1)
        self.assertEqual(next(c for c in r["checks"] if c["id"]==3)["status"],"FAIL")
        self.assertEqual(next(c for c in r["checks"] if c["id"]==9)["status"],"NOT_RUN")
    def test_human_scores_never_invented(self):
        raw={"sourceClosureSha256":H,"checks":{},"screens":[{"id":"real","png":{"path":"x.png","sha256":H,"width":240,"height":240,"sourceClosureSha256":H}}],"run":{},"detectorExit":0}
        r=adapter.normalize(raw,[],[])
        self.assertIsNone(r["screens"][0]["score"])
        self.assertTrue(r["screens"][0]["defects"])
if __name__=="__main__":unittest.main()
