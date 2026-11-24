"""Host fixture only: emit an ordered read-only byte image, never a shipping bank."""
import hashlib
import json
import struct
import sys
from pathlib import Path
from uuid import UUID

factory, archive, out = map(Path, sys.argv[1:])
if hashlib.sha256(archive.read_bytes()).hexdigest() != "ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8":
    raise ValueError("unapproved archive")
sys.path.insert(0, str(factory.resolve()))
from compile_bank import CATALOG, load_factory

catalog = json.loads(CATALOG.read_bytes())
ids = {field["identifier"]: field["id"] for field in catalog["controls"]}
for name in sorted(catalog["unknown_legacy_keys"]):
    ids[name] = len(ids)
if len(ids) > 256:
    raise ValueError("field domain")
manifest = []
data = bytearray(b"SJ6F\x01\x00\x80\x02")
for uid, (bank, rows) in sorted(load_factory(archive).items(), key=lambda item: item[1][0]["midiBankChangeNumber"]):
    for pos, row in enumerate(sorted(rows.values(), key=lambda r: r["position"])):
        if row["position"] != pos:
            raise ValueError("unexpected authored order")
        bn, pn = bank["name"].encode(), row["name"].encode()
        fields = row["parameterValues"]
        if not (0 < len(bn) <= 92 and 0 < len(pn) <= 92 and 0 < len(fields) <= 240):
            raise ValueError("record exceeds host bounds")
        midi, pc = bank["midiBankChangeNumber"], row["programChangeNumber"]
        data.extend(struct.pack("<HBBBBBB16s16s", len(manifest), midi, pos, pc, len(bn), len(pn), len(fields), UUID(uid).bytes, UUID(row["uid"]).bytes))
        data.extend(bn + pn)
        for field in fields:
            data.extend(struct.pack("<Bf", ids[field["identifier"]], field["value"]))
        manifest.append([len(manifest), str(UUID(uid)), midi, bank["name"], str(UUID(row["uid"])), pos, row["name"], pc])
doc = json.dumps({"schema":"dnbox.sj6.build9.fixed.identity.v1","records":manifest}, ensure_ascii=False, separators=(",",":"),sort_keys=True).encode()
if len(manifest) != 640 or hashlib.sha256(doc).hexdigest() != "e028d9fd9b1d7715f4402af76334f7475692d18406b58c14e4d89413a6ea9931":
    raise ValueError("detached identity manifest mismatch")
with out.open("xb") as f:
    f.write(data)
print(len(data),hashlib.sha256(data).hexdigest())
