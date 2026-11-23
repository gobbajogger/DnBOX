// SPDX-License-Identifier: GPL-3.0-only
// Read-only extraction of the glyph subset used by the custom startup splash.
// Reproduce from the pinned 94197fc8 Elite package:
// node extract-splash-font.cjs <package.fwsc> <test-only-felucca_font.h>
'use strict';

const packageSha = '95b3195f4767ef5df33b365197b01203873e5eec3d7fa497c73e5e509484a4a6';
const appSlotSha = '2de9d0bf09c05853d496c50b8b75d885517d52058fe9048b325e5a30547debf3';
const headerSha = '560f6416e3d91d9f754337917786601855f1dca7302efa3bc2367b06d3800862';

function extract(image, sha) {
    if (image.length !== 651732 || sha(image) !== packageSha) throw Error('Different Elite package');
    // tools/fm1pkg_make.py ufw(), flash_image(), sfc(): 20 marker bytes precede
    // flash.bin. Its 0x4000 SFC section has a 0x120-byte directory before app.bin.
    const start = 20 + 0x400 + 0x4000 + 0x120;
    const app = Uint8Array.from(image.subarray(start, start + 0x97400));
    for (let block = 0; block < app.length; block += 32) {
        let key = (0x980f ^ ((0x120 + block) >> 2)) & 0xffff;
        for (let i = 0; i < 32; ++i) {
            app[block + i] ^= key & 255;
            key = ((key << 1) ^ (key & 0x8000 ? 0x1021 : 0)) & 0xffff;
        }
    }
    if (sha(app) !== appSlotSha) throw Error('Corrupt app slot');
    const view = new DataView(app.buffer);
    const definitions = [['B',388252,'DnBOXH'], ['S',386320,'STILH']];
    let out = '/* SPDX-License-Identifier: GPL-3.0-only */\n' +
        '/* Splash-only crop-LZ glyphs unpacked from the distributed 94197fc8 Elite build.\n' +
        ' * DnBOX and STILL plus the H cap-height metrics. Other glyphs have zero width. */\n';
    for (const [face, offset, chars] of definitions) {
        if (app[offset] !== 16 || app[offset+1] !== 2 || app[offset+2] !== 32 || app[offset+3] !== 126)
            throw Error('Font descriptor ' + face);
        const [adv, widths, offsets, blob] = [4,8,12,16].map(n => view.getUint32(offset+n, true)-0x02000120);
        const outAdvance = Array(95).fill(0), outWidth = Array(95).fill(0), outOffset = Array(95).fill(0);
        const data = [], capRows = [];
        for (const ch of chars) {
            const gi = ch.charCodeAt(0)-32, width = app[widths+gi];
            outAdvance[gi] = app[adv+gi]; outWidth[gi] = width; outOffset[gi] = data.length;
            let ptr = blob + view.getUint16(offsets+2*gi, true);
            const rect = app[ptr++] | (app[ptr++]<<8) | (app[ptr++]<<16);
            const x0 = rect & 63, y0 = (rect >> 6) & 63, w = (rect >> 12) & 63, h = rect >> 18;
            const stride = (w+1)>>1, raw = [];
            if (x0+w>width || y0+h>16) throw Error('Invalid crop');
            while (raw.length < stride*h) {
                const token = app[ptr++];
                if (token < 128) {
                    for (let i = 0; i <= token; ++i) raw.push(app[ptr++]);
                } else {
                    const dist = app[ptr++];
                    if (!dist || dist>raw.length) throw Error('Invalid crop-LZ back-reference');
                    for (let i = 0; i < token-128+3; ++i) raw.push(raw[raw.length-dist]);
                }
            }
            if (raw.length !== stride*h) throw Error('Invalid crop-LZ output length');
            const pixels = new Uint8Array(16*width);
            for (let y = 0; y < h; ++y) for (let x = 0; x < w; ++x) {
                const byte = raw[y*stride+(x>>1)], alpha = x&1 ? byte&15 : byte>>4;
                pixels[(y0+y)*width+x0+x] = alpha;
                if (ch === 'H' && alpha >= 8) capRows.push(y0+y);
            }
            for (let y = 0; y < 16; ++y) for (let x = 0; x < width; x += 2)
                data.push((pixels[y*width+x]<<4) | (x+1<width ? pixels[y*width+x+1] : 0));
        }
        function array(name, values, type) {
            const lines = [];
            for (let i = 0; i < values.length; i += 20) lines.push('    '+values.slice(i,i+20).join(', ')+',');
            return 'static const '+type+' FONT_'+face+'_'+name+'['+values.length+'] = {\n'+lines.join('\n')+'\n};\n';
        }
        out += array('DATA',data,'uint8_t') + array('OFF',outOffset,'uint16_t') +
               array('ADV',outAdvance,'uint8_t') + array('BW',outWidth,'uint8_t');
        out += 'static const felucca_font_t FONT_'+face+' = {16, 2, 32, 126, '+
               ['ADV','BW','OFF','DATA'].map(s=>'FONT_'+face+'_'+s).join(', ')+', {'+
               Math.min(...capRows)+', '+Math.max(...capRows)+'}};\n';
    }
    if (sha(Uint8Array.from(out, ch=>ch.charCodeAt(0))) !== headerSha) throw Error('Unpacked font integrity');
    return out;
}

if (require.main === module) {
    const fs = require('node:fs'), crypto = require('node:crypto');
    const digest = a=>crypto.createHash('sha256').update(a).digest('hex');
    const result = extract(fs.readFileSync(process.argv[2]), digest);
    fs.writeFileSync(process.argv[3], result, {flag:'wx'});
}
module.exports = extract;
