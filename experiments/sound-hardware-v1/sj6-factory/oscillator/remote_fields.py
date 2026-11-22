"""Recover the two checked oscillator settings of PWM String Pad from Remote."""
import hashlib
import json
import subprocess

REMOTE_REV = "90cf5429a0aac5929c72d8abf387d70c399488ad"
PROGRAM = "4DBA5169-1A26-45B7-9A8C-BE968BB2EA36"
BANK = "84B1A8EB-BA34-4B8B-B3BE-6FE8CADB40DA"
FILES = {
    "web/product-site/remote/superj6.mjs": "429fd2c9e2de54745865ff159ebb4d6c29fe6564e9c59f2ff21c19c098049b6a",
    "evidence/remote-qualification-20261120/superj6-source.json": "318b809f7e15b4db0619c1a982c59608f7bd6c0b0aa6f5cd658f9592c5cd1ec6",
    "tools/check-superj6-draft-source.py": "9a720fbc2c1162ad78cfe88c37e8c9a564fd4289a3507d4ee616efe04d9ed2f5",
}


def load(repo, out, spec):
    data = {}
    for path, expected in FILES.items():
        raw = subprocess.check_output(["git", "-C", repo, "show", f"{REMOTE_REV}:{path}"])
        if hashlib.sha256(raw).hexdigest() != expected:
            raise ValueError(f"Remote source differs at {path}")
        data[path] = raw
    saved = json.loads(data["evidence/remote-qualification-20261120/superj6-source.json"])
    if (saved["archiveSha256"], saved["bank"], saved["program"], saved["matchedOriginalFields"],
            saved["owners"]) != (spec["archive_sha256"], BANK, PROGRAM, 13, 2):
        raise ValueError("Remote's original archive validation receipt differs")
    source = out / "source-fields.mjs"
    source.write_bytes(data["web/product-site/remote/superj6.mjs"])
    js = ("import {SUPERJ6_PROGRAM,superJ6Specs} from './source-fields.mjs';"
          "console.log(JSON.stringify({program:SUPERJ6_PROGRAM,owners:[0,1].map(slot=>"
          "Object.fromEntries(superJ6Specs.filter(s=>s.slot===slot).map(s=>[s.field,s.value])))}))")
    payload = json.loads(subprocess.check_output(["node", "--input-type=module", "-e", js], cwd=out))
    program, (a, b) = payload["program"], payload["owners"]
    selected = next(p for p in spec["programs"] if p["program_uid"] == PROGRAM)
    if (program["bankArchiveSha256"], program["bankId"], program["programId"],
            program["name"], program["bank"]) != (spec["archive_sha256"], selected["bank_uuid"],
            PROGRAM, selected["program"], selected["bank"]):
        raise ValueError("Remote fixture and S4 matrix disagree on the program")
    if len(a) != 13 or a != b:
        raise ValueError("Remote owners disagree on the original control values")
    return [(PROGRAM, layer, a[str(82 + layer)], a[str(78 + layer)], a[str(210 + layer)])
            for layer in range(2)]
