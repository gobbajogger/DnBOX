# Exact uploaded 906 result

Authenticated archive SHA-256 68ad8b5fa0adca8b5da2b03f9303e9321c6a20907f5945e3fe879bc6494009a4.
Full comparison executed using compare.py on exact 73749f55 Build 27 S1PK.
All 906 entries in 13 banks preserve original array order and authored
locations in the private catalog. Its SHA-256 is
82a0c480a736ef9ed70ec1dd85fc10e15165f8b3f98f41d268ae42e2e4929b20.

| Observation | Count |
|---|---:|
| Uploaded entries matching exact 144 ordered ID/binary32 pairs | 693 |
| Distinct matching content sequences | 674 |
| Upload entries not in existing pack (distinct sequences) | 213 |
| Old ordinals covered by uploaded content | 693 of 694 |
| Repeated uploaded content entries | 19 |
| Repeated uploaded UID occurrences | 24 |
| UID groups containing different content | 11 |
| Ambiguously matching uploaded rows / content groups | 38 / 19 |

Retain all uploaded entries, names, order and position identities even
when control words can share storage. Do not infer historic identity from
current UID or control-word match alone. One unique old content sequence
is absent from the upload: replacing the old pack without keeping that
content plus legacy identity can break exact recall/rollback. Catalog
provenance and versioned edit ownership remain required. Ordered native
commands only, not equivalence of upstream metadata or rendered DSP.
