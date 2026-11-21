// The exact v0.0.26 release packages parse with the main installer's own code:
// model identity, supported slot map, and bundled Bluetooth controller.
const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const vm = require('node:vm');

const html = fs.readFileSync(path.join(__dirname, '../install/index.html'), 'utf8');
const cat = JSON.parse(fs.readFileSync(path.join(__dirname, '../versions.json'), 'utf8'));
const v26 = cat.versions.find((v) => v.version === 'nightly-94197fc8-dev');
const RELEASE = {
  elite: { sha256: '95b3195f4767ef5df33b365197b01203873e5eec3d7fa497c73e5e509484a4a6', size: 651732, crc32: 'a5953c62' },
  pro: { sha256: 'ce67d560eebd03cb8b9087f7723b1e0b71fb7cca256184cdfdd55a8c27c708b8', size: 651730, crc32: 'd8957148' },
};

function slice(start, end) {
  const i = html.indexOf(start), j = html.indexOf(end, i);
  assert.ok(i >= 0 && j > i, start);
  return html.slice(i, j);
}
const ctx = { Uint8Array, TextDecoder, Error };
vm.runInNewContext(slice('const BLOCKS = 20', '\nfunction insertCtl(') +
  '\nthis.productOf=productOf;this.logicalImage=logicalImage;this.controllerOf=controllerOf;this.pkCrc32=pkCrc32;', ctx);

function loadPkg(which) {
  const candidates = [
    process.env['MAIN026_' + which.toUpperCase() + '_FWSC'],
    path.join(__dirname, 'fixtures/main-026', 'dnbox-x0x37-0.0.26-' + which + '.fwsc'),
  ].filter(Boolean);
  const p = candidates.find((f) => fs.existsSync(f));
  assert.ok(p, '0.0.26 ' + which + ' package not found: set MAIN026_' + which.toUpperCase() +
    '_FWSC to the v0.0.26 release asset (sha ' + RELEASE[which].sha256 + ')');
  return fs.readFileSync(p);
}

test('catalog 0.0.26 entry matches the v0.0.26 release table', () => {
  assert.equal(v26.source_commit, '94197fc839b00aff47b98ca02f3c6396e4a6eabf');
  for (const m of ['elite', 'pro']) {
    assert.equal(v26.models[m].sha256, RELEASE[m].sha256);
    assert.equal(v26.models[m].size, RELEASE[m].size);
    assert.ok(new URL(v26.models[m].pkg).pathname.includes(RELEASE[m].sha256));
  }
});

for (const m of ['elite', 'pro']) test('exact 0.0.26 ' + m + ' package is accepted', () => {
  const buf = loadPkg(m);
  assert.equal(buf.length, RELEASE[m].size);
  assert.equal(crypto.createHash('sha256').update(buf).digest('hex'), RELEASE[m].sha256);
  assert.equal((ctx.pkCrc32(new Uint8Array(buf), 0, buf.length) >>> 0).toString(16).padStart(8, '0'), RELEASE[m].crc32);
  assert.equal(ctx.productOf(buf), v26.models[m].product);
  const ctl = ctx.controllerOf(ctx.logicalImage(buf));
  assert.equal(ctl.length, v26.models[m].controller_size);
  assert.equal(crypto.createHash('sha256').update(ctl).digest('hex'), v26.models[m].controller_sha256);
});
