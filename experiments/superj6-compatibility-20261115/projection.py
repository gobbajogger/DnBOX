"""Explicit, incomplete approximation to our ORIGINAL feature proxy, not J6 DSP.

Pass an already inspected preset from the unchanged PR909 parser. This module
does not parse a bank, replace original bytes, infer enum laws, or emit DNPR.
Every decoded input is copied without mutation and classified. Exact raw-byte
restoration remains PR909's job. Target equations and domains below are choices
of this experiment, not discoveries about AudioKit parameter laws.
"""
from copy import deepcopy
from decimal import Decimal, localcontext
import math

PROFILE = "sj6x-original-proxy/1"
VERSIONS = frozenset(("1","1.0.2","1.0.4","1.0.5","1.0.6","1.0.8","1.0.9",
                     "1.1","1.2.1","1.3.2","1.5","1.6","1.7","1.8"))
CUTOFFS = frozenset(("neoncutoffLayer1","neonfilterCutoff"))
RESONANCES = frozenset(("neonresonanceLayer1","neonfilterRez"))

def project_preset(preset, *, acknowledge_approximation=False,
                   cutoff_source=None, resonance_source=None):
    """Return an incomplete source-preserving proxy patch, or ValueError.

    No original is changed, including on failure. Cutoff and resonance selection
    are mandatory policy choices. Both amp quartets are retained independently;
    app link flags, model/waveform/route enums and all effects remain unsupported.
    Patch values are finite Python floats; C API application explicitly rounds
    them to binary32. They are NOT exact app parameter values or timing laws.
    """
    if acknowledge_approximation is not True:
        raise ValueError("explicit approximation acknowledgement required")
    if not isinstance(cutoff_source,str) or not isinstance(resonance_source,str) or cutoff_source not in CUTOFFS or resonance_source not in RESONANCES:
        raise ValueError("explicit supported cutoff and resonance source choices required")
    if not isinstance(preset,dict) or not isinstance(preset.get("version"),str) or preset["version"] not in VERSIONS:
        raise ValueError("unobserved preset version; exact transport is still possible")
    entries = preset.get("parameterValues")
    if not isinstance(entries,list):
        raise ValueError("parameterValues must be an inspected list")
    values = {}
    for item in entries:
        if not isinstance(item,dict):
            raise ValueError("parameter entry must be an object")
        key, value = item.get("identifier"), item.get("value")
        if not isinstance(key,str) or not key or key in values:
            raise ValueError("missing or duplicate identifier")
        if isinstance(value,bool) or not isinstance(value,(int,Decimal)):
            raise ValueError("source numbers must come from exact Decimal/int inspection")
        if isinstance(value,Decimal) and not value.is_finite():
            raise ValueError("nonfinite source number")
        values[key] = value

    used = {}
    def read(name,hi="1"):
        if name not in values:
            raise ValueError("missing required approximate source field: "+name)
        x = Decimal(values[name])
        if not Decimal(0) <= x <= Decimal(hi):
            raise ValueError("unsupported source range, not clamped: "+name)
        used[name] = "APPROXIMATE experiment equation; actual J6 law unverified"
        return x
    def env(names):
        a,d,s,r = (read(n) for n in names)
        return {"attack":float(Decimal(".002")+3*a*a),
                "decay":float(Decimal(".01")+3*d*d),"sustain":float(s),
                "release":float(Decimal(".01")+4*r*r)}

    with localcontext() as ctx:
        ctx.prec = 60
        envelopes = [
            env(("neonattack","neondecay","neonsustain","neonrelease")),
            env(("neonattack2","neondecay2","neonsustain2","neonrelease2")),
            env(("neonfilterAttack","neonfilterDecay","neonfilterSustain","neonfilterRelease"))]
        c, r = read(cutoff_source), read(resonance_source)
        engine = {"envelopes":envelopes,
                  "cutoff_hz":float(35+12000*c*c),"resonance":float(r*Decimal(".6")),
                  "filter_env_hz":float(7000*read("neonfilterStrength")),
                  "saw":float(read("neonsawVolume","1.001")),
                  "pulse":float(read("neonpulseVolume","1.001")),
                  "sub":float(read("neonsubVolume")),
                  "noise":float(read("neonnoiseVolume")),
                  "level":float(read("neonvolumeControl","1.32")),
                  # Manual original-engine defaults, never claimed app defaults.
                  "highpass_hz":0.0,"pulse_width":[.5,.5],
                  "detune_cents":[0.0,0.0],"layer_mix":[.5,.5],
                  "lfo":[{"hz":0.0,"depth":0.0,"routes":0} for _ in range(3)]}
    if not all(math.isfinite(x) for x in (engine["cutoff_hz"],engine["resonance"],
                                         engine["filter_env_hz"],engine["level"])):
        raise ValueError("nonfinite target conversion")
    unsupported = {key:"preserved only; model/enum/unit/interaction not verified"
                   for key in values if key not in used}
    return {"profile":PROFILE,"status":"APPROXIMATE_NOT_J6_DSP",
            "projection_complete":False,"sound_equivalence":False,
            "source":deepcopy(preset),"source_selection":{
                "cutoff":cutoff_source,"resonance":resonance_source,
                "amp":"both quartets independently; link flag not interpreted"},
            "engine":engine,"mapped":used,"unsupported":unsupported,
            "manual_defaults":"two equal layers, no detune/LFO/HP/effects; NOT app defaults"}
