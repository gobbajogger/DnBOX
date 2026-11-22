#!/usr/bin/env python3
"""Boot an isolated 360-MHz SMK with the real optional dual-role BLE controller."""
import argparse
import gzip
from hashlib import sha256
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

def sha(data):
    return sha256(data).hexdigest()

def read(path):
    return path.read_bytes() if path.is_file() else gzip.decompress(path.with_suffix(path.suffix + '.gz').read_bytes())

def load_module(path):
    spec = importlib.util.spec_from_file_location("prepare_smk_test_project", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod

p = argparse.ArgumentParser(description=__doc__)
for key in ("repo", "emulator", "fwsc", "elf", "controller", "stage", "output"):
    p.add_argument(key, type=lambda v: Path(v).resolve())
p.add_argument('--ble', type=int, choices=(0, 1), default=1)
args=p.parse_args()
sys.path.insert(0, str(args.repo / "tools"))
from add_ctl import add
from fm1_rescue import package_flash_image
scene = load_module(args.repo / "experiments/sound-hardware-v1/selected-native/run.py")
fw = add(read(args.fwsc), read(args.controller))
out = args.output
out.mkdir(parents=True, exist_ok=True)
(out / 'work').mkdir(exist_ok=True)
(out / 'controller.fwsc').write_bytes(fw)
flash = bytearray(read(args.stage / 'flash.bin'))
protected = package_flash_image(fw)
assert len(flash) == 0x100000 and len(protected) == 0x9d000
flash[:len(protected)] = protected
objects = bytearray(read(args.stage / 'save-objects.bin'))
assert struct.unpack_from('<I', objects)[0] == 44
# First project blob header, format, then settings_t. Verify offsetof(settings_t, bt) with the helper.
objects[4 + 4 + 27] = (objects[4 + 4 + 27] & ~1) | args.ble
scene.install_project(flash, objects)
(out / 'flash.bin').write_bytes(flash)
syms = {}
for line in subprocess.check_output(['nm', '-n', args.elf], stderr=subprocess.DEVNULL, text=True).splitlines():
    row = line.split()
    if len(row) == 3:
        syms[row[2]] = int(row[0], 16)
env = {k:v for k,v in os.environ.items() if not k.startswith('FM1_')}
env.update(FM1_STRICT='1', FM1_DUMPRAM=str(out/'ram.bin'))
cmd = [args.emulator, '--fwsc', out/'controller.fwsc', '--mhz', '360',
       '--shot', '2500', '--flash', out/'flash.bin']
with (out/'emulator.log').open('w') as log:
    subprocess.run(cmd, env=env, cwd=out, stdout=log, stderr=subprocess.STDOUT,
                   timeout=360, check=True)
ram = (out/'ram.bin').read_bytes()
base = 0x01c00000
def byte(symbol):
    return ram[syms[symbol] - base]
def words(symbol, n):
    return list(struct.unpack_from('<' + 'I'*n, ram, syms[symbol] - base))
data = {k:byte(k) for k in ('ble_boot_on','ble_blocked','ble_c0_ready','ble_dead','ble_want','ble_up','ble_host_on') if k in syms}
assert data['ble_boot_on'] == data['ble_c0_ready'] == 1 and data['ble_blocked'] == data['ble_dead'] == 0
assert data['ble_want'] == args.ble
mb = words('ble_r', 12)
assert mb[:3] == [0x584d4258, 2, 0]
audio = words('x0x_dbg', 6)
assert audio[1] and audio[2] < 11609 and audio[4:6] == [0,0]
result = dict(firmware_sha256=sha(read(args.fwsc)), elf_sha256=sha(read(args.elf)),
              controller_sha256=sha(read(args.controller)), package_with_controller_sha256=sha(fw),
              emulator_sha256=sha(read(args.emulator)), staged_flash_sha256=sha(flash),
              saved_ble=args.ble, startup_state=data, mailbox_head=mb, audio=audio, guard=words('ble_guard',8))
(out / 'receipt.json').write_text(json.dumps(result, indent=2)+'\n')
