"""Compare public-slot PCM for direct AMP2-tags, supported stock attack and unedited notes."""
import argparse
import array
import hashlib
import json
import os
from pathlib import Path
import subprocess


def verify(out):
    encoded = {mode: (out / (mode + ".pcm")).read_bytes()
               for mode in ("unedited", "tagged-amp2", "stock-attack")}
    samples = {mode: array.array("i", data) for mode, data in encoded.items()}
    assert all(len(x) == 65536 and x.itemsize == 4 for x in samples.values())
    assert samples["unedited"] == samples["tagged-amp2"]
    assert samples["unedited"] != samples["stock-attack"]
    assert any(samples["unedited"])
    result = {}
    for mode, values in samples.items():
        changed = [i for i, (a,b) in enumerate(zip(samples["unedited"],values)) if a != b]
        result[mode] = {"stereo_frames": len(values)//2, "nonzero_words": sum(v != 0 for v in values),
                        "changed_words_vs_unedited": len(changed),
                        "first_changed_frame": changed[0]//2 if changed else None,
                        "sha256": hashlib.sha256(encoded[mode]).hexdigest()}
    (out / "results.json").write_text(json.dumps(result, indent=2) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("repo", type=Path)
    ap.add_argument("out", type=Path)
    args = ap.parse_args()
    repo, out = args.repo.resolve(), args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    entry = Path(__file__).resolve().with_name("entry.c")
    env = dict(os.environ, SMC_UI_TEST_FILE=str(entry), SMC_UI_OUT=str(out / "binary"),
               X0X_HOST_DEFS="-Itests/host", SJ6_ENTRY_CASE="unedited",
               SJ6_ENTRY_PCM=str(out / "unedited.pcm"))
    with (out / "build.log").open("w") as log:
        subprocess.run(["sh", "tests/host/smc_ui_test.sh"], cwd=repo, env=env,
                       check=True, stdout=log, stderr=subprocess.STDOUT)
    for mode in ("tagged-amp2", "stock-attack"):
        env.update(SJ6_ENTRY_CASE=mode, SJ6_ENTRY_PCM=str(out / (mode+".pcm")))
        subprocess.run([out / "binary/test"], cwd=repo, env=env, check=True)
    verify(out)


if __name__ == "__main__":
    main()
