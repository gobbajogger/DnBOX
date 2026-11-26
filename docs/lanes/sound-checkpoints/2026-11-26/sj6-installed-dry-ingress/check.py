#!/usr/bin/env python3
"""Pin the archived original AU runner's *explicit* dry write ordering."""
import hashlib
import json
import re
import sys
import tarfile

def selected(cap, arm):
    keys = {}
    for v in cap[arm]:
        addr = int(v["address"])
        if addr in (107,108,109):
            assert addr not in keys
            keys[addr] = v["value"]
    assert len(keys) == 3
    return keys

def check(path):
    with tarfile.open(path, "r:gz") as archive:
        name="./factory.swift"
        src = archive.extractfile(name).read().decode()
        plan = re.search(r'let ids:\[AUParameterAddress\]=\[([^]]+)\]', src)
        ids = list(map(int, plan.group(1).split(",")))
        assert ids[-1] == 107
        assert 108 not in ids and 109 not in ids
        before = src.index("let before=params()")
        writes = src[plan.end():before]
        assert 'if id==107 && p.value==0 {p.setValue(1,' in writes
        assert 'if p.value != 0 {p.setValue(0,' in writes
        assert 'wait(0.35,{false})' in writes
        assert 'setValue(' not in src[before:]
        path = "./program-1-dry/capture.json"
        cap = json.load(archive.extractfile(path))
        a,b,c = (selected(cap, s) for s in ("authored","active","after"))
        assert cap["wanted"]["program"] == "BASS: Solidify"
        assert b == c and a[108] == b[108] and a[109] == b[109]
        assert b[107] == 0
        print(f"sender_sha256={hashlib.sha256(src.encode()).hexdigest()}")
        print(f"capture_sha256_pcm={cap['pcm_sha256']}")
        print(f"before_explicit_dry={a}; at_render={b}; after_render={c}")
        print(f"explicit host dry order={ids}; last writer sends 107=0")
        print("No host #108/#109 write follows; no proof of hidden AU activity.")

if __name__ == "__main__":
    check(sys.argv[1])
