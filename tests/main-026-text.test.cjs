// Text fixes for the 0.0.26 main installer: neutral consent hint, public version
// in the status line, and HARDWARE UNTESTED for any row that is not a tested pin.
const test = require('node:test'), assert = require('node:assert/strict'), fs = require('node:fs'), vm = require('node:vm'), path = require('node:path');
const html = fs.readFileSync(path.join(__dirname, '../install/index.html'), 'utf8');
const cat = JSON.parse(fs.readFileSync(path.join(__dirname, '../versions.json'), 'utf8'));
const V26 = 'nightly-94197fc8-dev', V251 = 'nightly-79280e76-dev';
const row = (c, id) => c.versions.find((v) => v.version === id);
function fn(name) { const start = html.indexOf('function ' + name + '('); assert.ok(start >= 0, name); let i = html.indexOf('{', start), n = 1, j = i + 1; for (; n; j++) { if (html[j] === '{') n++; if (html[j] === '}') n--; } return html.slice(start, j); }
const pinSrc = html.slice(html.indexOf('const MAIN_251_SOURCE'), html.indexOf('function versions()'));
const TEXT = { hardwareuntested: 'HARDWARE UNTESTED', devchecked: 'DEV CHECKED', superseded: 'SUPERSEDED', otherbuilds: 'Other builds' };
function el() { return { children: [], className: '', textContent: '', append(...k) { this.children.push(...k); }, replaceChildren() { this.children = []; }, addEventListener() {} }; }
function texts(e) { return [e.textContent, ...e.children.flatMap(texts)]; }
function history(c) {
  const box = el();
  const ctx = { catalogLive: c, catalog: c, URL, document: { createElement: el }, $: () => box, t: (k) => TEXT[k] || k };
  vm.runInNewContext(pinSrc + '\n' + ['isRecord', 'versions', 'archivedCatalogVersions', 'catalogDenied', 'buildBadge', 'renderBuildHistory'].map(fn).join('\n') +
    '\nrenderBuildHistory();', ctx);
  return box.children.filter((a) => a.className === 'build-item').map((a) => ({ title: a.children[0].textContent, badges: a.children[1] ? texts(a.children[1]).filter(Boolean) : [] }));
}
function api(c) { const ctx = { catalogLive: c, URL }; vm.runInNewContext(fn('isRecord') + '\n' + pinSrc + '\nthis.label=statusVersionLabel;this.tested=testedByPin;', ctx); return ctx; }

test('(1) the install hint is neutral in both languages', () => {
  const hints = [...html.matchAll(/\n    hintack: "([^"]*)",/g)].map((m) => m[1]);
  assert.equal(hints.length, 2);
  for (const h of hints) assert.doesNotMatch(h, /開発版|development|DEV|untested|未テスト/i, h);
  assert.ok(hints.includes('Confirm the model and consent above to continue.'));
  assert.ok(hints.includes('上の機種と同意事項を確認し、チェックを入れてください。'));
});

test('(2) status line shows the display version only for pinned rows', () => {
  const a = api(cat);
  assert.equal(a.label(row(cat, V26)), '0.0.26');
  assert.equal(a.label(row(cat, V251)), '0.0.25.1');
  const c = structuredClone(cat); row(c, V26).source_commit = '600b3b45'; row(c, V26).display_version = '0.0.26';
  assert.equal(api(c).label(row(c, V26)), V26, 'an unpinned row keeps its catalog id');
  const other = cat.versions.find((v) => ![V26, V251].includes(v.version));
  assert.equal(a.label({ ...other, display_version: '9.9.9' }), other.version);
  assert.match(html, /say\(`DnBOX \$\{statusVersionLabel\(ver\)\} · /);
  assert.doesNotMatch(html, /say\(`DnBOX \$\{ver\.version\}/);
});

test('(3) only pins mark a row tested; unpinned hardware_tested:true still shows HARDWARE UNTESTED', () => {
  const c = structuredClone(cat);
  for (const v of c.versions) if (![V26, V251].includes(v.version)) v.hardware_tested = true;
  const a = api(c);
  assert.equal(a.tested(row(c, V26)), true);
  assert.equal(a.tested(row(c, V251)), false);
  for (const v of c.versions.filter((v) => ![V26, V251].includes(v.version))) assert.equal(a.tested(v), false, v.version);
  // Catalog rows render first; the later catalog.history items carry no badges.
  const all = history(c), rows = all.slice(0, c.versions.length);
  assert.equal(all.length, c.versions.length + (c.history || []).length);
  for (const r of rows) {
    const untested = r.badges.includes('HARDWARE UNTESTED');
    assert.equal(untested, !r.title.endsWith(' · ' + V26), r.title + ': ' + r.badges.join(', '));
  }
  // The live build panel uses the same pin rule, and no catalog flag check is left.
  assert.match(fn('renderBuildInfo'), /const ownerTested = testedByPin\(v\);[\s\S]*if\(!ownerTested\) b\.append\(buildBadge\(t\("hardwareuntested"\)/);
  assert.doesNotMatch(html, /v\.hardware_tested!==true/);
});
