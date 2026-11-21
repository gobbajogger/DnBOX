// SPDX-License-Identifier: GPL-3.0-only
// Run with node replay.cjs /absolute/path/to/picoc.asm.js [output.rgb565].
'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { createHash } = require('node:crypto');
const { gunzipSync } = require('node:zlib');

const interpreter = require.resolve(path.resolve(process.argv[2]));
const source = gunzipSync(fs.readFileSync(path.join(__dirname, 'picoc-input.c.gz'))).toString('utf8');
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');

function change(text, before, after) {
    assert.equal(text.split(before).length, 2, 'expected one source anchor');
    return text.replace(before, after);
}

function run(code) {
    // Each case starts in a new interpreter, with a fresh emulated filesystem.
    delete require.cache[interpreter];
    const pc = require(interpreter);
    let log = '';
    const append = ch => { if (ch != null) log += String.fromCharCode(ch); };
    pc.FS.init(() => null, append, append);
    let status = null;
    try {
        status = pc.picoc(code);
    } catch (error) {
        if (error !== 'longjmp') throw error;
    }
    const frame = pc.FS.analyzePath('/screen.bin').exists ? Buffer.from(pc.FS.readFile('/screen.bin')) : null;
    return {status, log, frame};
}

const noGuard = change(source, 'if (seq.pat == NULL)\n        return;', '');
const lostPage = change(source, 'memcpy(&sector0[0][write_count*256],src,n);',
    'if(write_count!=8)memcpy(&sector0[0][write_count*256],src,n);');
for (const [label, code, marker] of [['unguarded', noGuard, 'PATTERN_UNBOUND'], ['lost-page', lostPage, 'BAD 71']]) {
    const result = run(code);
    assert.notEqual(result.status, 0);
    assert.ok(result.log.includes(marker), label);
    assert.equal(result.frame, null);
    console.log(label, marker, 'no frame');
}

const result = run(source);
assert.equal(result.status, 0);
assert.match(result.log, /SPLASH writes=16 restored=1/);
assert.equal(result.frame.length, 240 * 240 * 2);
assert.equal(sha256(result.frame), '3f3295b59fadf90714afffd3760db7ecced667ac52e82e5352e9849e4f0b546a');
const rgb = Buffer.alloc(240 * 240 * 3);
for (let i = 0; i < 240 * 240; ++i) {
    const pixel = result.frame.readUInt16BE(i * 2);
    rgb[i * 3] = Math.floor((pixel >> 11) * 255 / 31);
    rgb[i * 3 + 1] = Math.floor(((pixel >> 5) & 63) * 255 / 63);
    rgb[i * 3 + 2] = Math.floor((pixel & 31) * 255 / 31);
}
assert.equal(sha256(rgb), 'af5f8a788579f1f844d3fcacd34782cb8fb55ae1a5d7c4c8a2efa053e521fb88');
if (process.argv[3]) fs.writeFileSync(process.argv[3], result.frame, {flag:'wx'});
console.log(result.log.trim(), sha256(rgb));
