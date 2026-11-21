#!/usr/bin/env python3
"""The corpus manifest, not a *.json glob, controls factory input enumeration."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]


class FactoryGeneratorTests(unittest.TestCase):
    def test_manifest_and_unlisted_json_are_not_treated_as_banks(self):
        with tempfile.TemporaryDirectory() as temp:
            repo = Path(temp)
            for name in ['tools/compile-synthone-factory.py', 'firmware/src/dsp/synthone_controls_data.inc',
                         'tests/fixtures/synthone-factory/frontend/Preset.swift',
                         'tests/fixtures/synthone-factory/frontend/PresetDataManager.swift']:
                dest = repo / name
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(REPO / name, dest)
            corpus = repo / 'assets/factory_banks/synthone'
            corpus.mkdir(parents=True)
            data = json.dumps([{'name': 'Synthetic default', 'uid': 'test'}]).encode()
            (corpus / 'bank.json').write_bytes(data)
            (corpus / 'MANIFEST.json').write_text(json.dumps({'files': [
                {'name': 'bank.json', 'sha256': hashlib.sha256(data).hexdigest()}]}))
            (corpus / 'unlisted.json').write_text('{"not_a_bank":true}')
            out = repo / 'out'
            subprocess.run([sys.executable, repo / 'tools/compile-synthone-factory.py', '--out', out],
                           check=True, capture_output=True, text=True)
            rows = json.loads((out / 'ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
            self.assertEqual(len(rows), 1)
            self.assertEqual(len(rows[0]['commands']), 144)
            self.assertEqual(rows[0]['bank'], 'bank.json')


if __name__ == '__main__':
    unittest.main()
