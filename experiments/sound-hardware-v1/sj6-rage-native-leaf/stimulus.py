"""Generate a synthetic known-input stereo compressor diagnostic. No app PCM."""
import argparse, hashlib, json, math, struct
from pathlib import Path

def plan(count, pattern):
    sizes = []
    while count:
        n = min(count, pattern[len(sizes) % len(pattern)])
        sizes.append(n)
        count -= n
    return sizes

def generate(path, rate):
    path.mkdir(parents=True, exist_ok=True)
    sections = [
        ("quiet_reference", 250, -42, -42),
        ("stereo_minus_24", 180, -24, -24),
        ("stereo_minus_18", 180, -18, -18),
        ("stereo_minus_12", 180, -12, -12),
        ("stereo_minus_6", 180, -6, -6),
        ("pre_transient", 250, -42, -42),
        ("left_sustained", 180, -2, -42),
        ("left_release", 300, -42, -42),
        ("right_sustained", 180, -42, -2),
        ("right_release", 300, -42, -42),
        ("true_silence", 100, None, None),
    ]
    planes = [bytearray(), bytearray()]
    spans, at = [], 0
    for name, duration, l, r in sections:
        count = duration * rate // 1000
        assert count * 1000 == duration * rate
        spans.append({"name":name,"from":at,"to":at+count,"L_peak_dBFS":l,"R_peak_dBFS":r})
        for channel, (db, hz) in enumerate(zip((l,r), (400,600))):
            amp = 0 if db is None else 10 ** (db / 20)
            planes[channel].extend(struct.pack("<%df" % count,*[amp*math.sin(math.tau*hz*i/rate) for i in range(at,at+count)]))
        at += count
    raw = b"".join(planes)
    rawfile = path / "input-planar.f32le"
    rawfile.write_bytes(raw)
    manifest = dict(
        schema="dnbox-synthetic-compressor-input-v1", source="synthetic; no app PCM",
        format="2 planar channels binary32 little endian, L plane followed by R",
        frames=at, rate=rate, tones_hz=[400,600], input_sha256=hashlib.sha256(raw).hexdigest(),
        tracks=spans,
        callback_schedules={"128":plan(at,(128,)), "mixed":plan(at,(17,31,63,128,251))},
        unit="aufx/dcmp/appl",
        parameters={"threshold_dB":-18.125,"headroom_dB":5,"attack_s":0.001,"release_s":0.05,"master_gain_dB":1.4375},
    )
    (path / "manifest.json").write_text(json.dumps(manifest,indent=2)+"\n")
    return manifest

if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("output",type=Path)
    p.add_argument("--rate",choices=(44100,48000),type=int,default=48000)
    args = p.parse_args()
    m=generate(args.output,args.rate)
    print(m["rate"],m["frames"],m["input_sha256"])
