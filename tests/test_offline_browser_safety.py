"""Exercise the actual browser runner's launch boundary and failure cleanup.

Only Playwright process creation is replaced; directory creation/cleanup and
the real runner execute. Removing sandbox/path checks or closing out of order
must fail before any real browser can be launched.
"""
import importlib.util
import io
import os
import tempfile
import types
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('offline_browser_runner', ROOT/'web/offline-sequencer/f6b-browser.test.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)

class InjectedFailure(RuntimeError):
    pass

class BrowserSafety(unittest.TestCase):
    def setUp(self):
        # The parallel host runner intentionally isolates TMPDIR inside its checkout.
        # Browser fixtures must use the same dedicated external root as the runner.
        root = os.environ.get('DNBOX_BROWSER_TMPDIR') or os.environ['TMPDIR']
        self.temp = tempfile.TemporaryDirectory(prefix='browser-contract-', dir=root)
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.events = []
        self.profile = None
        MODULE.OUT = self.base/'output'
        outer = self
        class Browser:
            def new_context(self, **kwargs):
                raise InjectedFailure('context creation fails')
            def close(self):
                outer.assertTrue(outer.profile.is_dir())
                outer.events.append('browser-close')
        class Playwright:
            def __enter__(self):
                outer.events.append('playwright-enter')
                return types.SimpleNamespace(chromium=types.SimpleNamespace(launch=outer.launch))
            def __exit__(self, *args):
                if outer.profile is not None:
                    outer.assertTrue(outer.profile.is_dir(), 'profile removed before Playwright exited')
                outer.events.append('playwright-exit')
        self.browser = Browser()
        self.patch = patch.object(MODULE, 'sync_playwright', Playwright)
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def launch(self, **options):
        self.assertIs(options.get('chromium_sandbox'), True, 'normal sandbox must be explicit')
        self.assertNotIn('executable_path', options)
        self.assertNotIn('channel', options)
        self.assertNotIn('ignore_default_args', options)
        self.assertNotIn('--no-sandbox', options.get('args', []))
        self.assertNotIn('--disable-setuid-sandbox', options.get('args', []))
        self.profile = Path(options['env']['TMPDIR']).resolve()
        self.assertTrue(self.profile.is_relative_to(self.base))
        self.assertTrue(self.profile.is_dir())
        self.events.append('launch')
        return self.browser

    def test_failure_closes_browser_then_playwright_then_profile(self):
        with patch.dict(os.environ, {'TMPDIR':str(self.base)}, clear=True):
            with self.assertRaises(InjectedFailure):
                MODULE.run()
        self.assertEqual(self.events, ['playwright-enter','launch','browser-close','playwright-exit'])
        self.assertFalse(self.profile.exists())

    def test_unsafe_temp_roots_fail_before_launch(self):
        link = self.base/'link-to-tmp'
        link.symlink_to('/tmp', target_is_directory=True)
        for root in ['', '/tmp', str(ROOT), str(link), str(self.base/'missing')]:
            with self.subTest(root=root), patch.dict(os.environ, {'TMPDIR':root}, clear=True):
                self.events.clear()
                with self.assertRaisesRegex(RuntimeError, 'TMPDIR'):
                    MODULE.run()
                self.assertNotIn('launch', self.events)

    def test_executable_overrides_fail_before_launch(self):
        for key in ['DNBOX_CHROMIUM_EXECUTABLE','CHROMIUM_EXECUTABLE','PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH']:
            with self.subTest(key=key), patch.dict(os.environ, {'TMPDIR':str(self.base),key:'/unapproved/browser'}, clear=True):
                self.events.clear()
                with self.assertRaisesRegex(RuntimeError, 'browser override'):
                    MODULE.run()
                self.assertNotIn('launch', self.events)

    def test_explicit_browser_temp_root_is_validated_and_bound(self):
        with patch.dict(os.environ, {'TMPDIR':'/tmp','DNBOX_BROWSER_TMPDIR':str(self.base)}, clear=True):
            with self.assertRaises(InjectedFailure):
                MODULE.run()
        self.assertFalse(self.profile.exists())
        with patch.dict(os.environ, {'TMPDIR':str(self.base),'DNBOX_BROWSER_TMPDIR':'/tmp'}, clear=True):
            with self.assertRaisesRegex(RuntimeError, 'TMPDIR'):
                MODULE.run()

    def test_contract_rejects_unsafe_runner_mutations(self):
        global MODULE
        original = MODULE
        source = Path(SPEC.origin).read_text()
        mutations = [
            ('chromium_sandbox=True', 'chromium_sandbox=False', 'test_failure_closes_browser_then_playwright_then_profile'),
            ('with closing(p.chromium.launch(', 'with __import__("contextlib").nullcontext(p.chromium.launch(', 'test_failure_closes_browser_then_playwright_then_profile'),
            ('with browser_temp_directory() as browser_tmp, sync_playwright() as p:', 'with sync_playwright() as p, browser_temp_directory() as browser_tmp:', 'test_failure_closes_browser_then_playwright_then_profile'),
            ("parent.is_relative_to(Path('/tmp').resolve())", 'False', 'test_unsafe_temp_roots_fail_before_launch'),
            ("if os.environ.get(key):", 'if False:', 'test_executable_overrides_fail_before_launch'),
        ]
        for before, after, method in mutations:
            with self.subTest(mutation=before):
                self.assertEqual(source.count(before), 1)
                mutant = types.ModuleType('mutant')
                mutant.__file__ = SPEC.origin
                exec(compile(source.replace(before, after), SPEC.origin, 'exec'), mutant.__dict__)
                MODULE = mutant
                try:
                    result = unittest.TextTestRunner(stream=io.StringIO()).run(unittest.TestSuite([BrowserSafety(method)]))
                    self.assertFalse(result.wasSuccessful(), 'unsafe mutation escaped contract')
                    self.assertFalse(result.errors, 'mutant must fail a safety assertion, not an unrelated runtime error')
                    self.assertTrue(result.failures)
                finally:
                    MODULE = original

if __name__ == '__main__':
    unittest.main()
