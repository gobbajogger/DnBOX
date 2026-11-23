#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import unittest

REPO=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('package',REPO/'tools/package-synthone-factory-pcm.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class PairGuards(unittest.TestCase):
    def test_694_actual_pairs_accepted(self):
        m.validate_pairs({f'preset-{i:02d}-{kind}.f32le.pcm' for i in range(694) for kind in ('compact','original')})

    def test_1388_names_with_unpaired_originals_rejected(self):
        names={f'preset-{i:02d}-{kind}.f32le.pcm' for i in range(693) for kind in ('compact','original')}
        names|={'preset-693-original.f32le.pcm','preset-694-original.f32le.pcm'}
        self.assertEqual(len(names),1388)
        with self.assertRaisesRegex(ValueError,'unpaired'):
            m.validate_pairs(names)

    def test_equal_counts_with_mismatched_pairs_rejected(self):
        names={f'preset-{i:02d}-{kind}.f32le.pcm' for i in range(694) for kind in ('compact','original')}
        names.remove('preset-693-original.f32le.pcm');names.add('preset-694-original.f32le.pcm')
        with self.assertRaisesRegex(ValueError,'unpaired'):
            m.validate_pairs(names)


if __name__=='__main__':unittest.main()
