"""Create the four declared excitation files once; no filter/synth processing."""
import hashlib
import json
import math
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parent
FRAMES = 32768
RATE = 44100


def f32_from_hex(bits):
    return struct.unpack("<f", struct.pack("<I", int(bits, 16)))[0]


def save(name, values, kind, amplitude_bits, frequency_hz):
    path = ROOT / "inputs" / name
    data = b"".join(struct.pack("<f", value) for value in values)
    if len(data) != FRAMES * 4:
        raise RuntimeError("unexpected input length")
    with path.open("xb") as stream:
        stream.write(data)
    return {
        "path": str(path), "bytes": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "kind": kind, "amplitude_f32_hex": amplitude_bits,
        "amplitude_f32": f32_from_hex(amplitude_bits),
        "frequency_hz": frequency_hz,
        "frequency_f32_hex": None if frequency_hz is None else "44fa0000",
        "frames": FRAMES, "sample_rate_hz": RATE,
        "generation": "Amplitude is the recorded binary32 value promoted to binary64; sine is evaluated in binary64 and each sample rounded once to binary32.",
    }


records = [save("impulse_a38d1b717.f32le",
                [f32_from_hex("38d1b717")] + [0.0] * (FRAMES - 1),
                "impulse", "38d1b717", None)]
for amplitude_bits in ("38d1b717", "3c23d70a", "3e800000"):
    amplitude = f32_from_hex(amplitude_bits)
    values = (amplitude * math.sin(2.0 * math.pi * 2000.0 * n / RATE)
              for n in range(FRAMES))
    records.append(save(f"sine_2000_a{amplitude_bits}.f32le", values,
                        "sine", amplitude_bits, 2000))
manifest = {
    "schema": "frozen standalone-filter excitation PCM v1",
    "format": "mono IEEE754 binary32 little endian",
    "status": "INPUT_STIMULI_ONLY; NO ORIGINAL FILTER EXECUTED",
    "records": records,
    "total_bytes": sum(record["bytes"] for record in records),
}
with (ROOT / "metadata" / "input_pcm_manifest.json").open("x") as stream:
    json.dump(manifest, stream, indent=2)
    stream.write("\n")
print(json.dumps({"input_count": len(records), "total_bytes": manifest["total_bytes"]}))
