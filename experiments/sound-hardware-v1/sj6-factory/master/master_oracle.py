"""Measure the original SuperJ6 master callback at its setVolume: boundary."""
import argparse
import bisect
import hashlib
import json
import math
import pathlib
import struct
import sys
import zipfile

def word(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]

def floating(bits):
    return struct.unpack("<f", struct.pack("<I", bits))[0]

def main():
    p = argparse.ArgumentParser()
    p.add_argument("bench", type=pathlib.Path)
    p.add_argument("banks", type=pathlib.Path)
    p.add_argument("output", type=pathlib.Path)
    args = p.parse_args()
    sys.path.insert(0, str(args.bench))
    from macho_dsp import DSP
    from unicorn.arm64_const import UC_ARM64_REG_X0, UC_ARM64_REG_X20

    raw_bank = args.banks.read_bytes()
    assert hashlib.sha256(raw_bank).hexdigest() == "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
    authored = []
    with zipfile.ZipFile(args.banks) as z:
        for bank in sorted(n for n in z.namelist() if n.startswith("J6presets/") and n.endswith("bank.json")):
            for preset in json.loads(z.read(bank))["presets"]:
                values = [k["value"] for k in preset["parameterValues"] if k["identifier"] == "neonvolumeControl"]
                assert len(values) == 1
                authored.append(word(values[0]))
    assert len(authored) == 640
    b = DSP()
    au = hashlib.sha256(b.image.b).hexdigest()
    assert au == "a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1"
    callback, scale = 0x1000ecef8, 0x1004902a8
    pow_stub = 0x1004902e0 + 1349560
    for address in (callback, scale):
        end = b.function_starts[bisect.bisect_right(b.function_starts, address)]
        b.allowed.append((address,end))
    b.allowed.append((pow_stub, pow_stub + 12))
    imported_pow = next(x['address'] for x in b.import_defs if x['name']=='_pow')
    b.imports[imported_pow] = lambda b: b.sd(0,math.pow(b.d(0),b.d(1)))
    # Use a minimal receiver layout so the original callback selects its actual
    # attenuator. Intercept ObjC lifetime calls and observe setVolume: arguments.
    offset_ptr = (0x1000ec000 + 7458816 + 2624)
    offset = struct.unpack("<Q",b.rd(offset_ptr,8))[0]
    assert offset < 65536
    conductor = b.alloc(offset+8)
    instrument = b.alloc(208)
    receiver = b.alloc(16)
    b.wr(conductor+offset, struct.pack("<Q",instrument))
    b.wr(instrument+200, struct.pack("<Q",receiver))
    calls = []
    def retain(b):
        assert b.uc.reg_read(UC_ARM64_REG_X0) == receiver
    def observe(b):
        retain(b)
        calls.append(b.d(0))
    b.imports[0x1005d99fc] = retain
    b.imports[0x1005d99f0] = retain
    b.imports[0x1005e6a20] = observe
    rows = []
    examples = [0., .25, .5, .75, 1., 1.32]
    for bits in sorted(set(authored)|{word(x) for x in examples}):
        b.uc.reg_write(UC_ARM64_REG_X20,conductor)
        b.sf(0,floating(bits))
        n = len(calls)
        b.call(callback,instructions=2000)
        assert len(calls) == n+1 and math.isfinite(calls[-1])
        rows.append([bits,word(calls[-1])])
    output = dict(original_au_sha256=au, bank_sha256=hashlib.sha256(raw_bank).hexdigest(),
        programs=len(authored), unique_factory_values=len(set(authored)), over_unity_programs=sum(floating(x)>1 for x in authored),
        greatest_raw=max(floating(x) for x in authored), observed_calls=len(calls),
        scope="Original AU callback through its setVolume: message. Object lifetime and libm pow are bridged. No Apple mixer output is substituted or recorded.",
        binary32_input_and_observed_binary32_gain=rows)
    args.output.write_text(json.dumps(output,indent=2)+"\n")
    print({k:v for k,v in output.items() if k not in ("binary32_input_and_observed_binary32_gain","scope")})
if __name__=="__main__": main()
