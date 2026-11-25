#!/usr/bin/env python3
"""Compare stereo original/compact dry and wet recordings without normalization."""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import tempfile
import wave

import numpy as np


FORMATS = ("wav", "f32le", "i16le")
KEYS = ("original-dry", "original-wet", "compact-dry", "compact-wet")
BANDS = (0, 60, 250, 500, 2000, 6000, 12000, 20000, math.inf)


def read_audio(path, fmt, rate):
    if fmt == "wav":
        with wave.open(str(path), "rb") as reader:
            if (reader.getnchannels(), reader.getsampwidth(), reader.getcomptype()) != (2, 2, "NONE"):
                raise ValueError(f"{path}: expected stereo PCM16 WAV")
            count = reader.getnframes()
            actual_rate = reader.getframerate()
            data = reader.readframes(count)
            if len(data) != count * 4:
                raise ValueError(f"{path}: truncated stereo frame data")
        if rate is not None and rate != actual_rate:
            raise ValueError(f"{path}: WAV rate {actual_rate} differs from --rate {rate}")
        rate = actual_rate
        dtype = "<i2"
    else:
        if rate is None:
            raise ValueError(f"{path}: raw stereo input requires --rate")
        dtype = "<f4" if fmt == "f32le" else "<i2"
        data = Path(path).read_bytes()
    size = np.dtype(dtype).itemsize * 2
    if not data or len(data) % size:
        raise ValueError(f"{path}: empty input or partial stereo frame")
    values = np.frombuffer(data, dtype=dtype).reshape(-1, 2).astype(np.float64)
    if dtype == "<i2":
        values /= 32768.0
    if not np.isfinite(values).all():
        raise ValueError(f"{path}: input contains NaN or infinity")
    return values, rate


def rms(x, axis=None):
    return np.sqrt(np.mean(x*x, axis=axis))


def db(value):
    return None if value <= 0 else float(20 * math.log10(value))


def delta_db(a, b):
    return None if a <= 0 or b <= 0 else db(b/a)


def shape_window(value, total, label, minimum):
    if value is None:
        return None
    try:
        start, stop = map(int, value.split(":"))
    except ValueError as error:
        raise ValueError(f"{label}: expected START:STOP") from error
    if not (0 <= start < stop <= total) or stop - start < minimum:
        raise ValueError(f"{label}: requires at least {minimum} frames inside [0, {total})")
    return start, stop


def signal_state(x, floor):
    peak = float(np.max(np.abs(x)))
    return {"peak": peak, "rms": float(rms(x)), "active": peak > floor}


def corr(x, floor):
    # Stereo Pearson correlation is descriptive only; the nulls use unmodified samples.
    lx, rx = x.T
    if min(np.max(np.abs(lx)), np.max(np.abs(rx))) <= floor:
        return {"value": None, "status": "silent-channel"}
    l = lx - np.mean(lx)
    r = rx - np.mean(rx)
    divisor = math.sqrt(float(l @ l) * float(r @ r))
    if divisor <= 0:
        return {"value": None, "status": "constant-channel"}
    return {"value": float(np.clip((l @ r) / divisor, -1, 1)), "status": "ok"}


def null_report(original, compact, floor):
    error = compact - original
    ref_level, other_level = signal_state(original, floor), signal_state(compact, floor)
    level = float(rms(error))
    status = "ok" if ref_level["active"] and other_level["active"] else (
        "both-silent" if not ref_level["active"] and not other_level["active"] else "activity-mismatch")
    c0, c1 = corr(original, floor), corr(compact, floor)
    c_delta = None if c0["value"] is None or c1["value"] is None else c1["value"] - c0["value"]
    return {
        "status": status,
        "original": ref_level,
        "compact": other_level,
        "null_rms": level,
        "null_dbfs": db(level),
        "null_peak": float(np.max(np.abs(error))),
        "null_lr_rms": rms(error, axis=0).tolist(),
        "null_relative_to_original_db": delta_db(ref_level["rms"], level),
        "stereo_pearson": {"original": c0, "compact": c1, "compact_minus_original": c_delta},
    }


def spectrum(original, compact, rate):
    # One fixed Hann window. Single-sided bin power, normalized for window power.
    n = len(original)
    taper = np.hanning(n)
    normalizer = n * float(taper @ taper)
    kb = np.fft.rfftfreq(n, 1/rate)
    weights = np.full(len(kb), 2.0)
    weights[0] = 1
    if n % 2 == 0:
        weights[-1] = 1
    powers = [np.abs(np.fft.rfft(x*taper[:, None], axis=0))**2 * weights[:, None] / normalizer
              for x in (original, compact)]
    rows = []
    for lo, hi in zip(BANDS, BANDS[1:]):
        idx = (kb >= lo) & (kb < hi)
        if not np.any(idx):
            continue
        levels = [math.sqrt(float(np.sum(p[idx], axis=0).mean())) for p in powers]
        rows.append({"start_hz": lo, "stop_hz": None if math.isinf(hi) else hi,
                     "bins": int(idx.sum()), "original_rms": levels[0], "compact_rms": levels[1],
                     "compact_minus_original_db": delta_db(*levels)})
    return rows


def compare(arrays, rate, shift=0, analysis=None, tail=None, envelope_ms=20.0, floor_db=-100.0,
            thresholds=None):
    if len(arrays) != 4 or len({x.shape for x in arrays}) != 1:
        raise ValueError("all four inputs must have identical frame count and stereo shape")
    if rate <= 0 or not math.isfinite(envelope_ms) or envelope_ms <= 0 or not math.isfinite(floor_db) or not (-300 <= floor_db <= 0):
        raise ValueError("expected positive rate, positive envelope duration and silence floor in [-300, 0] dBFS")
    n, channels = arrays[0].shape
    if channels != 2 or n-abs(shift) < 4:
        raise ValueError("alignment must leave at least four overlapping stereo frames")
    ro, co = max(0, -shift), max(0, shift)
    overlap = n - abs(shift)
    od, ow = (x[ro:ro+overlap] for x in arrays[:2])
    cd, cw = (x[co:co+overlap] for x in arrays[2:])
    ae, be = ow-od, cw-cd
    floor = 10**(floor_db / 20)
    a, b = shape_window(analysis, overlap, "analysis window", 4) or (0, overlap)
    # Tail is explicit: assuming that the last N frames follow note-off can hide
    # an early/late release or classify sustained input as a valid tail.
    t = shape_window(tail, overlap, "tail window", 4)
    out = {
        "rate_hz": rate, "input_frames": n, "aligned_frames": overlap,
        "alignment": {"compact_leading_samples": shift, "original_start_frame": ro, "compact_start_frame": co},
        "analysis_aligned_frames": [a,b], "silence_peak_dbfs": floor_db,
        "dry": null_report(od[a:b], cd[a:b], floor),
        "wet": null_report(ow[a:b], cw[a:b], floor),
        "effect_wet_minus_dry": null_report(ae[a:b], be[a:b], floor),
        "analysis_spectrum": {name:spectrum(x[a:b], y[a:b], rate)
                              for name,x,y in (("dry",od,cd),("effect_wet_minus_dry",ae,be))},
        "tail": {"status": "not-requested"},
    }
    if t:
        ta,tb=t
        count=max(1, round(min(tb-ta, rate*(envelope_ms/1000))))
        env=[]
        for i in range(ta,tb,count):
            j=min(i+count,tb)
            levels=[float(rms(x[i:j])) for x in (ae,be)]
            env.append({"aligned_frames":[i,j], "original_rms":levels[0], "compact_rms":levels[1],
                        "compact_minus_original_db":delta_db(*levels)})
        out["tail"]={"aligned_frames":[ta,tb],"envelope_block_frames":count,
                     **null_report(ae[ta:tb],be[ta:tb],floor),
                     "effect_spectrum":spectrum(ae[ta:tb],be[ta:tb],rate), "effect_envelope":env}
    limits={}
    for key,limit in (thresholds or {}).items():
        if not math.isfinite(limit) or not -300 <= limit <= 40:
            raise ValueError("max null threshold must be finite in [-300,40] dBFS")
        row=out[key]
        status="inconclusive" if row["status"]=="both-silent" else (
            "fail" if row["status"]=="activity-mismatch" or row["null_rms"]>10**(limit/20) else "pass")
        limits[key]={"max_null_dbfs":limit,"status":status}
    summary="report-only" if not limits else "fail" if any(x["status"]=="fail" for x in limits.values()) else (
        "inconclusive" if any(x["status"]=="inconclusive" for x in limits.values()) else "pass")
    out["caller_thresholds"]={"status":summary,"checks":limits}
    return out


def self_test():
    rate=8000
    t=np.arange(1024)/rate
    dry=0.125*np.column_stack([np.sin(2*np.pi*750*t),np.sin(2*np.pi*750*t)])
    dry[512:]=0
    effect=0.08*np.column_stack([np.sin(2*np.pi*350*t),-np.sin(2*np.pi*350*t)])*np.exp(-8*t[:,None])
    wet=dry+effect
    lim={"dry":-100,"effect_wet_minus_dry":-100}
    exact=compare([dry,wet,dry,wet],rate,tail="512:1024",thresholds=lim)
    assert exact["caller_thresholds"]["status"]=="pass"
    assert exact["dry"]["null_rms"]==0 and exact["effect_wet_minus_dry"]["null_dbfs"] is None
    assert exact["dry"]["stereo_pearson"]["original"]["value"]>0.999
    assert exact["tail"]["stereo_pearson"]["original"]["value"]< -0.999
    changed=dry + 0.5*effect
    diff=compare([dry,wet,dry,changed],rate,tail="512:1024",thresholds=lim)
    assert diff["caller_thresholds"]["status"]=="fail"
    assert diff["caller_thresholds"]["checks"]["dry"]["status"]=="pass"
    assert abs(diff["tail"]["effect_envelope"][0]["compact_minus_original_db"]+6.020599913)<1e-8
    band=next(x for x in diff["tail"]["effect_spectrum"] if x["start_hz"]==250)
    assert abs(band["compact_minus_original_db"]+6.020599913)<1e-8
    s=4
    d,w=[np.pad(x[:-s],((s,0),(0,0))) for x in [dry,wet]]
    assert compare([dry,wet,d,w],rate,s,thresholds=lim)["caller_thresholds"]["status"]=="pass"
    assert compare([dry,wet,d,w],rate,thresholds=lim)["caller_thresholds"]["status"]=="fail"
    assert compare([dry,dry,dry,dry],rate,thresholds=lim)["caller_thresholds"]["status"]=="inconclusive"
    assert compare([dry,wet,dry,dry],rate,thresholds=lim)["effect_wet_minus_dry"]["status"]=="activity-mismatch"
    with tempfile.TemporaryDirectory() as directory:
        d=Path(directory)
        ref=np.round(dry*32768).astype("<i2")
        (d/"audio.raw").write_bytes(ref.tobytes())
        with wave.open(str(d/"audio.wav"),"wb") as w:
            w.setparams((2,2,rate,0,"NONE","not compressed")); w.writeframes(ref.tobytes())
        (d/"float.raw").write_bytes((ref.astype("<f4")/32768).tobytes())
        rec=[read_audio(d/name,fmt,rate)[0] for name,fmt in zip(("audio.wav","audio.raw","float.raw"),("wav","i16le","f32le"))]
        assert all(np.array_equal(rec[0],x) for x in rec)
        (d/"float.raw").write_bytes(np.array([np.nan,0],dtype="<f4").tobytes())
        try:
            read_audio(d/"float.raw","f32le",rate)
        except ValueError as error:
            assert "NaN" in str(error)
        else:
            raise AssertionError("invalid signal accepted")
    return {"self_test":"PASS","scope":"synthetic only","exact_null_rms":exact["dry"]["null_rms"],
            "expected_half_tail_db":diff["tail"]["effect_envelope"][0]["compact_minus_original_db"],
            "checks":["exact","effect-only delta","tail spectrum","stereo","supplied alignment","silent null","silent mismatch","three formats","NaN rejection"]}


def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    for key in KEYS:
        p.add_argument("--"+key,type=Path)
        p.add_argument("--"+key+"-format",choices=FORMATS)
    p.add_argument("--format",choices=FORMATS)
    p.add_argument("--raw-format",choices=("f32","i16","f32le","i16le"),
                   help="all inputs are stereo interleaved raw (unless overridden per input)")

    p.add_argument("--rate",type=int)
    p.add_argument("--compact-leading-samples",type=int,default=0)
    p.add_argument("--window",help="analysis window START:STOP over the aligned overlap")
    tail_group=p.add_mutually_exclusive_group()
    tail_group.add_argument("--tail",help="caller-specified tail window START:STOP over aligned frames")
    tail_group.add_argument("--tail-start",type=float,help="tail start in seconds from aligned frame 0; ends at overlap end")

    p.add_argument("--envelope-ms",type=float,default=20)
    p.add_argument("--silence-peak-dbfs",type=float,default=-100)
    p.add_argument("--max-dry-null-dbfs",type=float)
    p.add_argument("--max-effect-null-dbfs",type=float)
    p.add_argument("--max-tail-null-dbfs",type=float)
    p.add_argument("--output",type=Path)
    p.add_argument("--self-test",action="store_true")
    args=p.parse_args(argv)
    try:
        if args.self_test:
            result=self_test()
        else:
            if any(getattr(args,k.replace("-","_")) is None for k in KEYS):
                p.error("all four inputs are required (or --self-test)")
            if args.raw_format and args.format:
                raise ValueError("--raw-format and --format are mutually exclusive")
            base_format=({"f32":"f32le","i16":"i16le"}.get(args.raw_format,args.raw_format)
                         if args.raw_format else args.format or "wav")
            reads=[]
            for k in KEYS:
                key=k.replace("-","_")
                reads.append(read_audio(getattr(args,key),getattr(args,key+"_format") or base_format,args.rate))
            rates={rate for _,rate in reads}
            if len(rates)!=1:
                raise ValueError("sample rates must match; the comparator never resamples")
            rate=rates.pop()
            if rate<=0:
                raise ValueError("sample rate must be positive")
            tail=args.tail
            if args.tail_start is not None:
                seconds=args.tail_start
                overlap=len(reads[0][0])-abs(args.compact_leading_samples)
                if not math.isfinite(seconds) or seconds<0 or seconds>=overlap/rate:
                    raise ValueError("--tail-start must lie inside the aligned overlap")
                tail=f"{round(seconds*rate)}:{overlap}"
            thresholds={k:v for k,v in (("dry",args.max_dry_null_dbfs),("effect_wet_minus_dry",args.max_effect_null_dbfs),("tail",args.max_tail_null_dbfs)) if v is not None}
            if "tail" in thresholds and not tail:
                raise ValueError("--max-tail-null-dbfs requires --tail or --tail-start")
            result=compare([x for x,_ in reads],rate,args.compact_leading_samples,
                           args.window,tail,args.envelope_ms,args.silence_peak_dbfs,thresholds)
        output=json.dumps(result,indent=2,allow_nan=False)+"\n"
        if args.output:
            args.output.write_text(output)
        else:
            print(output,end="")
        # The optional caller A/B results are informational, not a release gate.
        return 0
    except (OSError, ValueError, wave.Error, EOFError) as e:
        p.exit(2,f"compare: {e}\n")


if __name__=="__main__":
    raise SystemExit(main())
