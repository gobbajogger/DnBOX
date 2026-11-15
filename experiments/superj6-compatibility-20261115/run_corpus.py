"""Private-input qualification runner. Never writes bank/patch/audio bytes.
Dependencies: the exact unchanged PR909 parser/transport plus our own built proxy.
This proves preservation/proxy behavior, never SuperJ6 reference fidelity.
"""
import argparse, ctypes as C, hashlib, importlib, json, math, sys, zipfile
from collections import Counter
from pathlib import Path
from projection import project_preset
ARCHIVE_SHA="ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
DEPENDENCIES={"schema.py":"dc949084c01b1d377c53b7d9d6075a53d4a44ec4cf5763d649068256babda3ad",
              "mapping.py":"b3316e6ff8a18141b9d2aff874aacf04bfd9cb0f6411e0a6ce2733e1a4d68912"}
class ADSR(C.Structure):_fields_=[(x,C.c_float) for x in ("attack","decay","sustain","release")]
class LFO(C.Structure):_fields_=[("hz",C.c_float),("depth",C.c_float),("routes",C.c_uint)]
class Patch(C.Structure):
    _fields_=[("envelope",ADSR*3)]+[(x,C.c_float) for x in ("saw","pulse","sub","noise","level","cutoff_hz","resonance","filter_env_hz","highpass_hz")]+[(x,C.c_float*2) for x in ("pulse_width","detune_cents","layer_mix")]+[("lfo",LFO*3)]
class Env(C.Structure):_fields_=[("value",C.c_float),("release_step",C.c_float),("stage",C.c_uint)]
class Voice(C.Structure):
    _fields_=[("phase",C.c_float*2)]+[(x,C.c_float) for x in ("subphase","hz","velocity","filter_a","filter_b")]+[("envelope",Env*3),("note",C.c_uint),("gate",C.c_uint)]
class Engine(C.Structure):
    _fields_=[("patch",Patch),("voice",Voice*6),("sample_rate",C.c_float),("lfo_phase",C.c_float*3),("hp_input",C.c_float),("hp_state",C.c_float),("noise_state",C.c_uint32),("next_voice",C.c_uint),("tag",C.c_uint)]
def physical_patch(e):
    p=Patch()
    for i,v in enumerate(e["envelopes"]):
        p.envelope[i]=ADSR(*(v[k] for k in ("attack","decay","sustain","release")))
    for k in ("saw","pulse","sub","noise","level","cutoff_hz","resonance","filter_env_hz","highpass_hz"):setattr(p,k,e[k])
    for k in ("pulse_width","detune_cents","layer_mix"):getattr(p,k)[:]=e[k]
    for i,v in enumerate(e["lfo"]):p.lfo[i]=LFO(v["hz"],v["depth"],v["routes"])
    return p
def run(archive,reference,library):
    blob=archive.read_bytes()
    if hashlib.sha256(blob).hexdigest()!=ARCHIVE_SHA:raise ValueError("wrong private archive identity")
    reference=reference.resolve()
    for name,digest in DEPENDENCIES.items():
        if hashlib.sha256((reference/name).read_bytes()).hexdigest()!=digest:raise ValueError("wrong PR909 dependency")
    if "schema" in sys.modules or "mapping" in sys.modules:raise ValueError("run in clean process, no module substitution")
    sys.path.insert(0,str(reference));schema=importlib.import_module("schema");mapping=importlib.import_module("mapping")
    lib=C.CDLL(str(library.resolve()))
    lib.sj6x_init.argtypes=[C.POINTER(Engine),C.c_float]
    lib.sj6x_apply.argtypes=[C.POINTER(Engine),C.POINTER(Patch)]
    lib.sj6x_note_on.argtypes=[C.POINTER(Engine),C.c_uint,C.c_uint]
    lib.sj6x_note_off.argtypes=[C.POINTER(Engine),C.c_uint]
    lib.sj6x_render.argtypes=[C.POINTER(Engine),C.POINTER(C.c_float),C.c_uint]
    versions=Counter();unsupported=Counter();uids=[];banks=count=projections=samples=0;peak=0.;different_envelopes=0
    with zipfile.ZipFile(archive) as z:
        for name in sorted(z.namelist()):
            if name.startswith("__MACOSX/") or not name.endswith(".json"):continue
            raw=z.read(name);inspection=schema.inspect_bank(raw)
            if mapping.restore_bank(mapping.map_bank(raw))!=raw:raise AssertionError("raw bank restoration")
            banks+=1
            for p in inspection.data["presets"]:
                count+=1;versions[str(p["version"])]+=1;uids.append(str(p["uid"]))
                before=repr(p)
                for cutoff in ("neoncutoffLayer1","neonfilterCutoff"):
                    for resonance in ("neonresonanceLayer1","neonfilterRez"):
                        q=project_preset(p,acknowledge_approximation=True,cutoff_source=cutoff,resonance_source=resonance);projections+=1
                        if q["source"]!=p or repr(p)!=before:raise AssertionError("source mutation")
                        keys={v["identifier"] for v in p["parameterValues"]}
                        if set(q["mapped"])&set(q["unsupported"]) or keys!=set(q["mapped"])|set(q["unsupported"]):raise AssertionError("incomplete classification")
                q=project_preset(p,acknowledge_approximation=True,cutoff_source="neoncutoffLayer1",resonance_source="neonresonanceLayer1")
                unsupported.update(q["unsupported"].keys())
                e=Engine();patch=physical_patch(q["engine"])
                if not lib.sj6x_init(C.byref(e),48000) or not lib.sj6x_apply(C.byref(e),C.byref(patch)):raise AssertionError("proxy patch admission")
                if q["engine"]["envelopes"][0]!=q["engine"]["envelopes"][1]:different_envelopes+=1
                for note in (48,55,60,64,67,72):
                    if not lib.sj6x_note_on(C.byref(e),note,127):raise AssertionError("note admission")
                out=(C.c_float*256)()
                for block in range(4):
                    if not lib.sj6x_render(C.byref(e),out,256):raise AssertionError("proxy render")
                    for x in out:
                        if not math.isfinite(x) or abs(x)>8:raise AssertionError("nonfinite/unbounded original proxy")
                        peak=max(peak,abs(x));samples+=1
                for note in (48,55,60,64,67,72):
                    if not lib.sj6x_note_off(C.byref(e),note):raise AssertionError("note off")
    if (banks,count,projections,different_envelopes)!=(12,640,2560,29):raise AssertionError("unexpected corpus census")
    return dict(scope="EXACT raw restoration plus APPROXIMATE original proxy; NOT app fidelity",archive_sha256=ARCHIVE_SHA,
                banks=banks,presets=count,policy_choice_projections=projections,conflicting_amp_quartets_preserved=different_envelopes,
                versions=dict(versions),classified_unsupported_ids=len(unsupported),proxy_samples=samples,proxy_peak=peak,
                ordered_uid_sha256=hashlib.sha256("\n".join(uids).encode()).hexdigest(),host_engine_bytes=C.sizeof(Engine),host_patch_bytes=C.sizeof(Patch),
                failures=0,raw_or_audio_written=False)
if __name__=="__main__":
    p=argparse.ArgumentParser();p.add_argument("--archive",type=Path,required=True);p.add_argument("--reference",type=Path,required=True);p.add_argument("--library",type=Path,required=True)
    a=p.parse_args();print(json.dumps(run(a.archive,a.reference,a.library),indent=2,sort_keys=True))
