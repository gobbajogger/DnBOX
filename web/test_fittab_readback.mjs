// SPDX-License-Identifier: GPL-3.0-only
// SMK FITTAB (fit-table sector) in the browser installer: fm1pkg.packageFittab finds the package's
// fittab.bin entry, fm1ota.fittabReadback accepts only the package's header from a firmware that
// reports the region present.
// Run: node web/test_fittab_readback.mjs LOGICAL_WITH_FITTAB LOGICAL_WITHOUT  (tests/host/fittab_package_test.py)
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { packageFittab } from "./fm1pkg.js";
import { fittabReadback } from "./fm1ota.js";

const withF = new Uint8Array(readFileSync(process.argv[2]));
const without = new Uint8Array(readFileSync(process.argv[3]));
const ft = packageFittab(withF);
assert.ok(ft && ft.length === 0x1000, "fittab entry found, 4 KiB");
assert.equal(packageFittab(without), null, "no entry: null");
const bad = withF.slice();
bad[bad.length - 0x100] ^= 1;                       // inside fittab.bin (the last entry)
assert.throws(() => packageFittab(bad), /fittab\.bin: CRC/);

function link(reply) {
  const q = [];
  return {
    drain() { q.length = 0; },
    send(m) { assert.deepEqual(m, [0xF0, 0x7D, 0x46, 0x4C, 51, 0xF7]); const r = reply(); if (r) q.push(r); return true; },
    async read() { return q.shift() || null; },
  };
}
const frame = (ok, hdr) => {
  const body = [ok];
  for (const b of hdr) body.push(b & 0x7F, b >> 7);
  return new Uint8Array([0xF0, 0x7D, 0x46, 0x4C, 51, ...body, 0xF7]);
};
const hdr = Array.from(ft.slice(0, 16));
assert.deepEqual(await fittabReadback(link(() => frame(1, hdr)), ft, 5, 1), hdr);
const wrong = hdr.slice(); wrong[9] ^= 0x80;
await assert.rejects(fittabReadback(link(() => frame(1, wrong)), ft, 5, 1), (e) => e.code === "fittab");
await assert.rejects(fittabReadback(link(() => frame(0, hdr)), ft, 5, 1), (e) => e.code === "fittab");  // absent
await assert.rejects(fittabReadback(link(() => null), ft, 5, 2), (e) => e.code === "fittab");           // old firmware
console.log("fittab read back (web): entry found, CRC checked, only the package's header accepted ok");
