#!/usr/bin/env python3
"""Labelled component diagnostics from saved matched input/output streams.

No alignment, rescaling, fitted gain, signal regeneration or old dry-gate metric.
"""
from pathlib import Path
import hashlib
import json
import math

import numpy as np

HERE = Path(__file__).resolve().parent
RATE, FRAMES, STEADY = 44100, 32768, 16384


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def rms(values):
    return float(np.sqrt(np.mean(values * values)))


def db_ratio(numerator, denominator):
    return 20 * math.log10(numerator / denominator) if numerator > 0 and denominator > 0 else None


def projection(values, frequency, start, window):
    # Fixed-frequency direct spectral measurement; no parameter estimation or
    # waveform modification. Shared window factors cancel in reported ratios.
    indices = np.arange(start, start + len(values), dtype=np.float64)
    return np.sum(values * window * np.exp(-2j * np.pi * frequency * indices / RATE))


def sine_diagnostics(x, reference, candidate):
    window = np.hanning(FRAMES - STEADY)
    fundamental_input = projection(x[STEADY:], 2000, STEADY, window)
    result = {}
    phasors = {}
    for name, values in [("reference", reference), ("candidate", candidate)]:
        harmonics = [projection(values[STEADY:], 2000 * k, STEADY, window) for k in range(1, 6)]
        transfer = harmonics[0] / fundamental_input
        phasors[name] = transfer
        result[name] = {"fundamental_transfer_gain_db": db_ratio(abs(harmonics[0]), abs(fundamental_input)),
            "fundamental_transfer_phase_degrees": float(np.angle(transfer, deg=True)),
            "harmonic_2_to_5_db_relative_to_fundamental": {
                str(k): db_ratio(abs(harmonics[k - 1]), abs(harmonics[0])) for k in range(2, 6)},
            "combined_harmonics_2_to_5_db_relative_to_fundamental": db_ratio(
                math.sqrt(sum(abs(h) ** 2 for h in harmonics[1:])), abs(harmonics[0]))}
    difference = phasors["candidate"] / phasors["reference"]
    result["candidate_minus_reference_fundamental_gain_db"] = db_ratio(abs(difference), 1)
    result["candidate_minus_reference_fundamental_phase_degrees"] = float(np.angle(difference, deg=True))
    result["method"] = "Fixed2000Hz and harmonics2-5 direct projections on16384:32768, symmetric Hann; ratios only, no waveform gain/phase fitting. This is not a full THD measurement."
    return result


def impulse_diagnostics(x, reference, candidate):
    window = np.ones(FRAMES)
    rows = []
    for frequency in [0, 500, 1000, 2000, 4000, 8000, 16000, 22050]:
        x_line = projection(x, frequency, 0, window)
        ref = projection(reference, frequency, 0, window) / x_line
        new = projection(candidate, frequency, 0, window) / x_line
        ratio = new / ref
        rows.append({"frequency_hz": frequency,
            "reference_gain_db": db_ratio(abs(ref), 1),
            "candidate_gain_db": db_ratio(abs(new), 1),
            "candidate_minus_reference_gain_db": db_ratio(abs(ratio), 1),
            "reference_phase_degrees": float(np.angle(ref, deg=True)),
            "candidate_phase_degrees": float(np.angle(new, deg=True)),
            "candidate_minus_reference_phase_degrees": float(np.angle(ratio, deg=True))})
    return {"method": "Whole32768-sample unwindowed fixed-frequency response ratios for exact shared impulse; no alignment or fit", "rows": rows}


def main():
    declaration = json.loads((HERE / "DECLARATION.json").read_text())
    capture = json.loads((HERE / "CAPTURE-RESULT.json").read_text())
    require(capture["response_records"] == 8 and capture["status"] == "CAPTURE_COMPLETE",
            "Eight complete records required")
    results = []
    for case in declaration["cases"]:
        x_path, ref_path, y_path = map(Path, [case["input_path"], case["reference_path"], case["candidate_output_path"]])
        require(sha(x_path) == case["input_sha256"] and sha(ref_path) == case["reference_sha256"],
                "Shared numeric input/reference changed")
        receipt = next(row for row in capture["rows"] if row["case_id"] == case["case_id"])
        require(sha(y_path) == receipt["candidate_sha256"], "Candidate output changed")
        x, ref, candidate = [np.fromfile(p, dtype="<f4").astype(np.float64) for p in [x_path, ref_path, y_path]]
        require(all(len(v) == FRAMES and np.isfinite(v).all() for v in [x, ref, candidate]), "Nonfinite or malformed stream")
        row = {key: case[key] for key in ["case_id", "input_kind", "input_amplitude_f32", "input_amplitude_f32_hex",
            "resonance_f32", "resonance_f32_hex", "input_path", "input_sha256", "reference_path", "reference_sha256",
            "candidate_output_path"]}
        row["candidate_sha256"] = receipt["candidate_sha256"]
        row["windows"] = {}
        for name, section in [("whole", slice(0, FRAMES)), ("steady", slice(STEADY, FRAMES))]:
            xx, rr, yy = x[section], ref[section], candidate[section]
            x_rms, r_rms, y_rms = rms(xx), rms(rr), rms(yy)
            row["windows"][name] = {"input_rms": x_rms, "reference_rms": r_rms, "candidate_rms": y_rms,
                "reference_rms_gain_db": db_ratio(r_rms, x_rms),
                "candidate_rms_gain_db": db_ratio(y_rms, x_rms),
                "candidate_minus_reference_rms_db": db_ratio(y_rms, r_rms),
                "direct_unshifted_difference_rms": rms(yy - rr),
                "direct_unshifted_max_absolute_difference": float(np.max(np.abs(yy - rr)))}
        row["reference_peak_abs"] = float(np.max(np.abs(ref)))
        row["candidate_peak_abs"] = float(np.max(np.abs(candidate)))
        row["reference_peak_frame"] = int(np.argmax(np.abs(ref)))
        row["candidate_peak_frame"] = int(np.argmax(np.abs(candidate)))
        row["last4096_reference_rms"] = rms(ref[-4096:])
        row["last4096_candidate_rms"] = rms(candidate[-4096:])
        row["diagnostic"] = sine_diagnostics(x, ref, candidate) if case["input_kind"] == "sine" else impulse_diagnostics(x, ref, candidate)
        wrapper = receipt["wrapper_result"]
        row["feedback_argument_classification"] = wrapper["feedback_argument_classification"]
        for name in ["reconstructed_all", "reconstructed_steady"]:
            values = wrapper[name]
            row[name] = {**values, "rms": math.sqrt(values["sum_squares"] / values["count"])}
        results.append(row)
    result = {"schema": "matched frozen C filter-only numeric observations v1",
        "status": "DIAGNOSTIC_COMPLETE_NO_ACCEPTANCE_THRESHOLD", "response_records": 8,
        "protocol": declaration["protocol"], "analysis": declaration["diagnostic_method"],
        "declaration_sha256": sha(HERE / "DECLARATION.json"),
        "capture_result_sha256": sha(HERE / "CAPTURE-RESULT.json"),
        "total_candidate_nonfinite": 0, "rows": results}
    with (HERE / "reports" / "OBSERVATIONS.json").open("x") as stream:
        json.dump(result, stream, indent=2, allow_nan=False); stream.write("\n")
    for row in results:
        whole, steady = row["windows"]["whole"], row["windows"]["steady"]
        print(json.dumps({"case": row["case_id"], "reference_whole_gain_db": whole["reference_rms_gain_db"],
            "C_whole_gain_db": whole["candidate_rms_gain_db"], "reference_steady_gain_db": steady["reference_rms_gain_db"],
            "C_steady_gain_db": steady["candidate_rms_gain_db"], "C_minus_reference_steady_rms_db": steady["candidate_minus_reference_rms_db"],
            "reconstructed_steady_feedback_peak": row["reconstructed_steady"]["peak_abs"],
            "C_harmonics2_to5_db": row["diagnostic"].get("candidate", {}).get("combined_harmonics_2_to_5_db_relative_to_fundamental")}, allow_nan=False))


if __name__ == "__main__":
    main()
