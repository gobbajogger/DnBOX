import hashlib
import json
from pathlib import Path
import tempfile
import unittest

import numpy as np

from compare import SPEC, capture, channel_score, compare


class ComparisonTests(unittest.TestCase):
    def test_identity_gain_and_frequency(self):
        rng = np.random.default_rng(734)
        source = rng.normal(0, 0.1, 44100).astype("<f4")
        level, spectral = channel_score(source, source, 44100, [20, 16000])
        self.assertEqual((level, spectral), (0., 0.))
        level, _ = channel_score(source, source*.5, 44100, [20, 16000])
        self.assertAlmostEqual(level, -6.0206, places=4)
        quiet = np.zeros_like(source)
        self.assertEqual(channel_score(source, quiet, 44100, [20, 16000]), (None, None))
        # Preserve level while removing high frequencies.
        filtered = np.convolve(source, np.ones(9)/9, mode="same")
        filtered *= np.linalg.norm(source)/np.linalg.norm(filtered)
        level, spectral = channel_score(source, filtered, 44100, [20, 16000])
        self.assertLess(abs(level), 0.01)
        self.assertGreater(spectral, 3.)

    def test_missing_captures_and_wrong_record(self):
        spec = json.loads(SPEC.read_bytes())
        spec_hash = hashlib.sha256(SPEC.read_bytes()).hexdigest()
        row = spec["programs"][0]
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp)
            self.assertEqual(compare(target, target)["scored"], 0)
            rec = {"arm": "original", "bank_uuid": row["bank_uuid"],
                   "program_uid": row["program_uid"], "record_sha256": "wrong"}
            (target / (row["program_uid"] + ".json")).write_text(json.dumps(rec))
            with self.assertRaisesRegex(ValueError, "record_sha256"):
                capture(target, row, spec, spec_hash, "original")

    def test_qualified_pair_and_evidence_refusals(self):
        # Mock the independent release manifest for a generated test-only record.
        # Never emit a receipt claiming this sound is an actual factory capture.
        import copy
        from unittest.mock import patch
        from compare import control_words
        from factory import canonical_bytes

        spec = json.loads(SPEC.read_bytes())
        matrix_hash = hashlib.sha256(SPEC.read_bytes()).hexdigest()
        row = dict(spec["programs"][0])
        uid = row["program_uid"]
        def sha(b):
            return hashlib.sha256(b).hexdigest()

        record = {"uid": uid, "seqTempo": 84, "keyboardOctave": 2, "modWheelDestination": 0,
                  "microtonalMasterSet": [1.0, 1.5],
                  "parameterValues": [{"identifier": "testControl", "value": 0.375}]}
        row["record_sha256"] = sha(canonical_bytes(record))
        packed = canonical_bytes({"bank": {"uuid": row["bank_uuid"]}, "preset": record}) + b"\n"
        state = canonical_bytes(record)
        test_au = sha(b"test AU")
        commit, image = "1"*40, sha(b"candidate test image")
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            inputs = base/"inputs"
            inputs.mkdir()
            (inputs/(uid+".json")).write_bytes(packed)
            audit = {"au_sha256": test_au, "component_version": 1, "spec_sha256": matrix_hash,
                     "programs": [{"uid": uid, "file_sha256": sha(packed), "parameter_count": 1,
                                   "supplied_bank_uuid": row["bank_uuid"], "bundled_bank_uuid": "bundled"}]}
            audit_path = base/"audit.json"
            audit_path.write_text(json.dumps(audit))
            rng = np.random.default_rng(37)
            samples = rng.uniform(-.3,.3,(spec["capture_frames"],2)).astype("<f4")
            audio = samples.tobytes()
            rec = {"bank_uuid": row["bank_uuid"], "program_uid": uid, "record_sha256": row["record_sha256"],
                   "s4_inputs_sha256": matrix_hash, "frames": spec["capture_frames"], "format": "stereo-f32le",
                   "complete_preset": True, "first_event_frame": 0, "first_render_frame": 0,
                   "event_count": len(spec["events"]), "scheduled_events": spec["events"],
                   "authored_tempo": 84, "applied_tempo": 84, "input_file_sha256": sha(packed),
                   "applied_state_sha256": sha(state), "pcm_sha256": sha(audio),
                   "applied_readback": {"qualified": True, "authored": 1, "live_matches": 1,
                       "missing_active": [], "active_differences": [], "missing_live": [],
                       "live_differences": [], "unexposed_legacy": [], "live_words": control_words(record["parameterValues"])}}
            rec.update({k: spec[k] for k in ("channels","sample_rate","block_frames","gain","transport","reset")})

            with patch("compare.AUDIT", audit_path):
                for arm in ("original", "dnbox"):
                    directory = base/arm
                    directory.mkdir()
                    armrec = dict(rec, arm=arm)
                    if arm == "original":
                        armrec.update(au_sha256=test_au, release_au_sha256=test_au, component_version=1,
                                      factory_route="fullState", bundled_bank_uuid="bundled")
                    else:
                        armrec.update(source_commit=commit, firmware_sha256=image)
                    def read(receipt, pcm=audio, active=state, candidate=image, revision=commit):
                        (directory/(uid+".json")).write_text(json.dumps(receipt))
                        (directory/(uid+".f32le")).write_bytes(pcm)
                        (directory/(uid+".applied.json")).write_bytes(active)
                        return capture(directory, row, spec, matrix_hash, arm, inputs, candidate, revision)
                    self.assertEqual(read(armrec)[1], sha(audio))
                    # The release route regenerates these bytes from the checked IPA.
                    self.assertEqual(capture(directory, row, spec, matrix_hash, arm, {uid: packed},
                                             image, commit)[1], sha(audio))
                    for field, value in [("program_uid","other"),("record_sha256","other"),("pcm_sha256","wrong"),
                                         ("complete_preset",1),("gain",True),("first_render_frame",1),
                                         ("input_file_sha256","wrong"),("applied_tempo",120),
                                         ("scheduled_events",spec["events"][1:]),("applied_state_sha256","wrong")]:
                        with self.subTest(arm=arm, field=field), self.assertRaises(ValueError):
                            read(dict(armrec, **{field:value}))
                    for changes in [{"qualified":False},{"live_words":{}},{"live_matches":2},{"missing_live":["testControl"]},
                                    {"live_words":{"testControl":17}},{"unexposed_legacy":["testControl"]}]:
                        report = dict(armrec["applied_readback"], **changes)
                        with self.subTest(arm=arm,changes=changes), self.assertRaises(ValueError):
                            read(dict(armrec, applied_readback=report))
                    for changes in [{"uid":"other"},{"seqTempo":85},{"microtonalMasterSet":[1.0]},
                                    {"parameterValues":[{"identifier":"testControl","value":.25}]}]:
                        raw = canonical_bytes(dict(record, **changes))
                        with self.subTest(arm=arm, state=changes), self.assertRaises(ValueError):
                            read(dict(armrec, applied_state_sha256=sha(raw)), active=raw)
                    silent = np.zeros_like(samples)
                    silent_channel = samples.copy()
                    silent_channel[:,1] = 0.
                    nan = samples.copy()
                    nan[123,1] = np.nan
                    for label, pcm in [("empty",b""),("truncated",audio[:-4]),("silent",silent.tobytes()),
                                       ("nonfinite",nan.tobytes())]:
                        with self.subTest(arm=arm,pcm=label), self.assertRaises(ValueError):
                            read(dict(armrec, pcm_sha256=sha(pcm)), pcm=pcm)
                    pcm = silent_channel.tobytes()
                    with self.assertRaisesRegex(ValueError, "silent channel"):
                        read(dict(armrec, pcm_sha256=sha(pcm)), pcm=pcm)
                    if arm == "dnbox":
                        for candidate, revision in [(None,commit),(image,"vague"),(image,"2"*40),("other",commit)]:
                            with self.subTest(candidate=candidate,revision=revision), self.assertRaises(ValueError):
                                read(armrec,candidate=candidate,revision=revision)
                    else:
                        for field in ("au_sha256","component_version","factory_route","bundled_bank_uuid"):
                            with self.subTest(arm=arm,field=field), self.assertRaises(ValueError):
                                read(dict(armrec, **{field:"other"}))
                self.assertEqual(channel_score(silent[:,0],silent[:,1],44100,spec["band_hz"]), (0.0,0.0))
                self.assertEqual(channel_score(samples[:,0],silent[:,1],44100,spec["band_hz"]), (None,None))
                altered = copy.deepcopy(json.loads(packed))
                altered["preset"]["seqTempo"] = 101
                (inputs/(uid+".json")).write_bytes(canonical_bytes(altered))
                with self.assertRaisesRegex(ValueError, "input file digest"):
                    read(armrec)


if __name__ == "__main__":
    unittest.main()
