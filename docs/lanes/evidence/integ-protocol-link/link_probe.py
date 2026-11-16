"""Unexecuted code-only native linkage diagnostic; never endpoint admission."""
import hashlib, json, os, subprocess, sys, types
from pathlib import Path

HERE = Path(__file__).resolve().parent
manifest = json.loads((HERE / 'inputs.json').read_text())
ROOTS = ['dnr2_init','dnr2_changed','dnr2_tick','dnr2_handle','dnr2_decode','dnr2_encode_command','dnr2_encode_reply','dnr2_receive']
assert manifest['entry_points'] == ROOTS
root = Path.cwd().resolve()
model = os.environ['MODEL']
assert model in ('elite', 'pro')
assert os.environ.get('BOARD') == 'smk37'
assert os.environ.get('X0X_DEV_PADCAL') == '1'
def git(*args):
    return subprocess.check_output(['git', *args], cwd=root).decode().strip()
assert git('rev-parse', 'HEAD') == manifest['source']
assert not git('status', '--porcelain', '--untracked-files=no')
build_path = root / 'tools/build.py'
captured_build = build_path.read_bytes()
assert hashlib.sha256(captured_build).hexdigest() == manifest['build_py_sha256']
captured = {x['name']: (HERE / x['name']).read_bytes() for x in manifest['objects']}
for x in manifest['objects']:
    assert hashlib.sha256(captured[x['name']]).hexdigest() == x['sha256']
out = root / 'build' / ('smk37-elite' if model == 'elite' else 'smk37')
elf = out / 'x0x.elf'
baseline = elf.read_bytes()
assert hashlib.sha256(baseline).hexdigest() == manifest["baseline_elf_sha256"][model]
evidence = root / 'build' / ('protocol-probe-' + model)
evidence.mkdir(exist_ok=False)
(evidence / 'baseline.elf').write_bytes(baseline)
assert (out / 'x0x.map').is_file(), 'map-only baseline required'
(evidence / 'baseline.map').write_bytes((out / 'x0x.map').read_bytes())
staged = []
for name, data in captured.items():
    p = evidence / name
    p.write_bytes(data)
    staged.append(p)
def check_inputs():
    assert git('rev-parse', 'HEAD') == manifest['source']
    assert not git('status', '--porcelain', '--untracked-files=no')
    assert build_path.read_bytes() == captured_build
    for p in staged:
        assert p.read_bytes() == captured[p.name]

sys.path.insert(0, str(root / 'tools'))
build = types.ModuleType('native_build')
build.__file__ = str(build_path)
exec(compile(captured_build, str(build_path), 'exec'), build.__dict__)
original = build.tc
links = []
def tc(*args, **kwargs):
    if args and args[0] == 'pi32v2/bin/ld' and '-o' in args:
        target = Path(args[args.index('-o') + 1]).resolve()
        if target.name == 'x0x.elf':
            assert target == elf.resolve() and not links, 'unexpected app link'
            check_inputs()
            added = tuple(staged) + tuple('--undefined=' + x for x in manifest['entry_points']) + ('-Map', evidence / 'probe.map')
            links.append({'original': list(map(str, args)), 'added': list(map(str, added))})
            return original(*args, *added, **kwargs)
    return original(*args, **kwargs)
build.tc = tc
sys.argv = ['build.py', '--board', 'smk37', '--model', model]
completed = False
result = None
failure = None
raised_primary = False
try:
    result = build.main()
    completed = result is None or result == 0
except BaseException as exc:
    failure = exc
    raised_primary = True
    raise
finally:
    inputs_ok = False
    try:
        check_inputs()
        inputs_ok = True
    except BaseException as exc:
        if failure is None:
            failure = exc
    target_ok = build.BOARD == 'smk37' and build.MODEL == model and build.OUT.resolve() == out
    report = {'source': manifest['source'], 'model': model, 'build_completed': completed,
              'build_return': result, 'primary_error': None if failure is None else str(failure),
              'input_checks_pass': inputs_ok, 'target_checks_pass': target_ok,
              'status': 'LINKED_DIAGNOSTIC' if len(links) == 1 and completed and inputs_ok and target_ok else 'NOT_QUALIFIED',
              'baseline_elf_sha256': hashlib.sha256(baseline).hexdigest(), 'links': links,
              'environment': {k: os.environ.get(k) for k in ('MODEL','BOARD','X0X_DEV_PADCAL','X0X_CDC','X0X_DEBUG','X0X_WOBBLE','X0X_DEV_WOBBLE','X0X_LCD_TRACE','X0X_BASS_PRESET_PREVIEW','X0X_BASS_TAGGED_SOUND','X0X_BASS_NOTE_SOUND')},
              'compiler_sha256': hashlib.sha256((Path(os.environ['JIELI_TOOLCHAIN'])/'pi32v2/bin/clang').read_bytes()).hexdigest(),
              'sdk_boot_sha256': hashlib.sha256((Path(os.environ['AC79_SDK'])/'cpu/wl82/tools/uboot.boot').read_bytes()).hexdigest(),
              'scope': 'linked code-only diagnostic: excludes core allocation, catalog/glue/transport, stack, IRQ latency and execution; not endpoint fit'}
    missing = ROOTS[:]
    if len(links) == 1 and elf.exists():
        after = elf.read_bytes()
        (evidence / 'probe.elf').write_bytes(after)
        report['probe_elf_sha256'] = hashlib.sha256(after).hexdigest()
        symbols = subprocess.check_output(['readelf', '-sW', str(elf)], text=True)
        missing = [name for name in ROOTS if not any(line.split()[-1:] == [name] and ' UND ' not in line for line in symbols.splitlines())]
    else:
        report['status'] = 'NOT_RUN_APP_LINK'
    report['missing_retained_symbols'] = missing
    if missing:
        report['status'] = 'NOT_QUALIFIED'
    (evidence / 'receipt.json').write_text(json.dumps(report, indent=2) + '\n')
    if failure is not None and not raised_primary:
        raise failure
    if failure is None:
        assert completed and inputs_ok and target_ok and len(links) == 1 and not missing, report
