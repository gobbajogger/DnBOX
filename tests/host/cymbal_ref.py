#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Prepare independent IMA decodes for the shipping stream and the dated A/B stream."""
import argparse
import hashlib
import re
import struct
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(SRC / "tools"))
import sampleio as sio  # noqa: E402
import gen_drum_samples as gds  # noqa: E402

CYM = [("hh", "hh.wav"), ("ride", "ride.wav"), ("crash", "crash.wav")]

# Stream identities at sound PR #973 head 63278e3, before the offline trellis encoder.
HISTORICAL = {
    "hh": "050eca307e59d71f74160077f14a55e791d708d26b33d310204a94e03443fac4",
    "ride": "cdc9d47cc5116c0fb6d0672ed934fc04f84800554e3b32abb146d035b4543b18",
    "crash": "17c62887552760606ab73a64d33881f6e784d84a4e8cb5c7dc79df52d529bad1",
}


def decode(b, n):
    pred, idx, out = 0, 0, []
    for i in range(n):
        code = (b[i >> 1] >> (4 if i & 1 else 0)) & 15
        step = sio.IMA_STEP[idx]
        vd = step >> 3
        if code & 4:
            vd += step
        if code & 2:
            vd += step >> 1
        if code & 1:
            vd += step >> 2
        pred = max(-32768, min(32767, pred - vd if code & 8 else pred + vd))
        idx = max(0, min(88, idx + sio.IMA_IDX[code & 7]))
        out.append(pred)
    return out


def wav_pcm(path):
    d = path.read_bytes()
    ch = sio.wav_chunks(d)
    o, n = ch[b"data"]
    return list(struct.unpack_from("<%dh" % (n // 2), d, o))


def header_adpcm(c):
    hdr = (SRC / "build" / "gen" / "x0x_drum_samples.h").read_text()
    m = re.search(r"x0x_smp_%s\[(\d+)\] = \{(.*?)\};" % c, hdr, re.S)
    n = int(re.search(r"X0X_SMP_%s_LEN (\d+)u" % c.upper(), hdr).group(1))
    return bytes(int(v) for v in re.findall(r"\d+", m.group(2))), n


def write_wavs(out):
    out.mkdir(parents=True, exist_ok=True)
    for c, fn in CYM:
        data, n = header_adpcm(c)
        pcm = struct.pack("<%dh" % n, *decode(data, n))
        (out / fn).write_bytes(b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVEfmt " +
                               struct.pack("<IHHIIHH", 16, 1, 1, 44100, 88200, 2, 16) + b"data" +
                               struct.pack("<I", len(pcm)) + pcm)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--wav", action="store_true")
    mode.add_argument("--historical", action="store_true")
    parser.add_argument("output", type=Path)
    parser.add_argument("ab_dir", type=Path, nargs="?")
    args = parser.parse_args()
    out = args.output
    if args.wav:
        write_wavs(out)
        return
    ab = args.ab_dir
    if args.historical and ab is None and (SRC.parent / "adpcm_ab").is_dir():
        ab = SRC.parent / "adpcm_ab"
    if ab is not None and not args.historical:
        parser.error("the dated A/B comparison requires --historical")
    out.mkdir(parents=True, exist_ok=True)
    header = ["#pragma once", "#include <stdint.h>"]
    for c, fn in CYM:
        pcm, _ = gds.read_wav(gds.ASSETS / fn)
        n = len(pcm)
        if args.historical:
            data, _ = sio.ima_encode(pcm, 0)
            if hashlib.sha256(data).hexdigest() != HISTORICAL[c]:
                sys.exit(f"{c}: historical A/B encoder drifted")
            header.extend([f"#define X0X_SMP_{c.upper()}_LEN {n}u",
                           gds.arr("uint8_t", f"x0x_smp_{c}", data, per=24)])
        else:
            data, frames = header_adpcm(c)
            if frames != n:
                sys.exit(f"{c}: source/header length mismatch")
        if ab is not None:
            if data != (ab / f"{c}.adpcm").read_bytes():
                sys.exit(f"{c}: historical ADPCM differs from {ab}")
            ref = wav_pcm(ab / f"{c}_2_adpcm44k.wav")
        else:
            ref = decode(data, n)
        if len(ref) != n:
            sys.exit(f"{c}: expected {n} reference samples")
        (out / f"{c}.ref").write_bytes(struct.pack(f"<{n}h", *ref))
        (out / f"{c}.pcm").write_bytes(struct.pack(f"<{n}h", *pcm))
    if args.historical:
        (out / "x0x_drum_samples.h").write_text("\n".join(header))
        print("Historical A/B stream digests matched.", f"External decode: {ab or 'unavailable'}")
    else:
        print("Production trellis streams decoded independently.")


if __name__ == "__main__":
    main()
