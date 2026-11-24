// SPDX-License-Identifier: GPL-3.0-only
// SMK XDATA (font region) in the browser installer: fm1pkg.packageXdata finds the package's xdata.bin entry,
// fm1ota.xdataReadback accepts only the package's header from a firmware that uses the region.
// Run: node web/test_xdata_readback.mjs LOGICAL_WITH_XDATA LOGICAL_WITHOUT  (tests/host/xdata_package_test.py)
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { packageXdata } from "./fm1pkg.js";
import { xdataReadback } from "./fm1ota.js";

const withX = new Uint8Array(readFileSync(process.argv[2]));
const without = new Uint8Array(readFileSync(process.argv[3]));
const xd = packageXdata(withX);
assert.ok(xd && xd.length === 0x2000, "xdata entry found, 8 KiB");
assert.equal(packageXdata(without), null, "no entry: null");
const bad = withX.slice();
bad[bad.length - 0x1000] ^= 1;                      // inside xdata.bin (the last entry)
assert.throws(() => packageXdata(bad), /xdata\.bin: CRC/);

function link(reply) {                              // reply(header bytes) -> SysEx frame or null
  const q = [];
  return {
    drain() { q.length = 0; },
    send(m) { assert.deepEqual(m, [0xF0, 0x7D, 0x46, 0x4C, 50, 0xF7]); const r = reply(); if (r) q.push(r); return true; },
    async read() { return q.shift() || null; },
  };
}
const frame = (ok, hdr) => {
  const body = [ok];
  for (const b of hdr) body.push(b & 0x7F, b >> 7);
  return new Uint8Array([0xF0, 0x7D, 0x46, 0x4C, 50, ...body, 0xF7]);
};
const hdr = Array.from(xd.slice(0, 16));
assert.deepEqual(await xdataReadback(link(() => frame(1, hdr)), xd, 5, 1), hdr);
const wrong = hdr.slice(); wrong[9] ^= 0x80;        // another CRC / build id in flash
await assert.rejects(xdataReadback(link(() => frame(1, wrong)), xd, 5, 1), (e) => e.code === "xdata");
await assert.rejects(xdataReadback(link(() => frame(0, hdr)), xd, 5, 1), (e) => e.code === "xdata");   // FONT_T fallback
await assert.rejects(xdataReadback(link(() => null), xd, 5, 2), (e) => e.code === "xdata");            // old firmware: silence
console.log("xdata read back (web): entry found, CRC checked, only the package's header accepted ok");
