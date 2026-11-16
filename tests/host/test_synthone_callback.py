#!/usr/bin/env python3
import csv
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('callback', Path(__file__).resolve().parents[2] / 'tests/host/run_synthone_callback.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


class DeadlineTrace(unittest.TestCase):
    def write(self, root, change=None, count=750):
        path = root / 'trace.csv'
        with path.open('w') as f:
            writer = csv.writer(f)
            writer.writerow(['preset', 'block', 'frames', 'rate', 'wall_ns', 'cpu_ns', 'wall_miss', 'cpu_miss'])
            for i in range(count):
                row = [0, i, 128, 48000, 2666666, 100, 0, 0]
                if change and i == 0: change(row)
                writer.writerow(row)
        return path

    def test_exact_fractional_deadline_boundary(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.assertEqual(m.summarize(self.write(root), 1)['wall_misses'], 0)
            def change(row): row[4], row[6] = 2666667, 1
            summary = m.summarize(self.write(root, change), 1)
            self.assertEqual(summary['wall_misses'], 1)
            self.assertEqual(summary['cpu_misses'], 0)
            self.assertEqual(summary['frames'], 96000)

    def test_incomplete_trace_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, 'incomplete'):
                m.summarize(self.write(Path(d), count=749), 1)

    def test_wrong_order_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, 'identity/order'):
                m.summarize(self.write(Path(d), lambda row: row.__setitem__(1, 99)), 1)

    def test_relaxed_deadline_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, 'deadline changed'):
                m.summarize(self.write(Path(d), lambda row: row.__setitem__(3, 44100)), 1)

    def test_false_zero_miss_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, 'count mismatch'):
                m.summarize(self.write(Path(d), lambda row: row.__setitem__(4, 3000000)), 1)

    def test_changed_orchestration_tool_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'factory.py'
            path.write_bytes(b'original')
            snapshot = {path: path.read_bytes()}
            m.verify_tool_snapshot(snapshot)
            path.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'orchestration tool changed'):
                m.verify_tool_snapshot(snapshot)

    def test_benchmark_not_parent_tracer(self):
        self.assertEqual(m.parse_benchmark_tracer('runner_tracer_pid=0\nbenchmark_tracer_pid=123 (observed)\n'), 123)
        self.assertEqual(m.parse_benchmark_tracer('benchmark_tracer_pid=-1 (unavailable)\n'), -1)

    def test_missing_duplicate_benchmark_tracer_rejected(self):
        for text in ('runner_tracer_pid=0\n', 'benchmark_tracer_pid=0\nbenchmark_tracer_pid=0\n'):
            with self.assertRaisesRegex(ValueError, 'benchmark tracer'):
                m.parse_benchmark_tracer(text)

    def test_negative_time_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, 'negative'):
                m.summarize(self.write(Path(d), lambda row: row.__setitem__(5, -1)), 1)


if __name__ == '__main__': unittest.main()
