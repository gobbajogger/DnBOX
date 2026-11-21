// SPDX-License-Identifier: GPL-3.0-only
// Audit the generated native-field ownership and bounds used in the wave reachability proof.
// Run from the repo root: node tests/ui_inventory/wave_preview_domain.cjs
'use strict';

function audit(read) {
    function requireCondition(test, message) {
        if (!test) throw new Error(message);
    }
    function initializer(source, name) {
        const match = source.match(new RegExp('\\b' + name + '\\[\\d+\\]\\s*=\\s*\\{([\\s\\S]*?)\\};'));
        if (!match) throw new Error('Missing generated table ' + name);
        return match[1];
    }
    function numbers(source, name) {
        return initializer(source, name).split(',').map(s => s.trim()).filter(Boolean)
            .map(s => {
                requireCondition(/^(?:0x[\da-f]+|\d+)u?$/i.test(s), 'Unexpected integer in ' + name);
                return Number(s.replace(/u$/i, ''));
            });
    }
    // Only exact binary fractions 0..1 from the generated float literals are needed here.
    function hexFloat(token) {
        const parts = token.trim().match(/^0x([01])\.([0-9a-f]+)p([+-]?\d+)f$/i);
        requireCondition(parts, 'Unexpected mapping coefficient ' + token);
        return (Number(parts[1]) + parseInt(parts[2], 16) / 16 ** parts[2].length) * 2 ** Number(parts[3]);
    }
    const dir = 'experiments/original_synth/';
    const dictionary = read(dir + 'program_values_compact.h');
    const words = numbers(dictionary, 'original_program_value_words');
    function fixedValue(kind, id) {
        const index = numbers(dictionary, 'compact_' + kind + '_base_codes')[id];
        requireCondition(index < words.length, 'Invalid compact base ' + kind + '/' + id);
        const buf = new DataView(new ArrayBuffer(4));
        buf.setUint32(0, words[index]);
        return buf.getFloat32(0);
    }
    function mappings(kind) {
        const prefix = 'original_' + kind + '_';
        const source = read(dir + kind + '_program_private.h');
        const starts = numbers(source, prefix + 'macro_start');
        const rows = Array.from(initializer(source, prefix + 'mappings').matchAll(/\{([^{}]+)\}/g),
            match => match[1].split(',').map(s => s.trim()));
        const lookup = initializer(source, prefix + 'lookup').split(',').map(hexFloat);
        const byId = new Map();
        requireCondition(starts[0] === 0 && starts[starts.length-1] === rows.length, 'Incomplete target slices');
        for (let owner = 0; owner + 1 < starts.length; ++owner) {
            requireCondition(starts[owner] <= starts[owner+1], 'Overlapping target slices');
            for (let i = starts[owner]; i < starts[owner+1]; ++i) {
                const [id, type, length, offset, levels, outer, a, b] = rows[i];
                requireCondition(!byId.has(+id), 'Duplicate native target ' + id);
                byId.set(+id, {owner, type:+type, length:+length, offset:+offset, levels:+levels,
                    outer:+outer, a:hexFloat(a), b:hexFloat(b)});
            }
        }
        return {byId, lookup};
    }
    function fixed(kind, table, id) {
        requireCondition(!table.byId.has(id), 'Expected constant ' + kind + '/' + id);
        return fixedValue(kind, id);
    }
    const chicago = mappings('chicago');
    const chicagoShapes = [1,9].map(id => Math.floor(fixed('chicago', chicago, id) * 4 + 0.5));
    requireCondition(chicagoShapes.every(shape => shape >= 0 && shape <= 3), 'Chicago shape unsupported');
    const gains = [3,11].map(id => {
        const row = chicago.byId.get(id);
        requireCondition(row && row.owner === 0 && [0,1].includes(row.type) && row.outer === 0 &&
            row.levels > 1 && Math.min(row.a,row.b) >= 0 && Math.max(row.a,row.b) <= 1,
            'Unexpected Chicago gain mapping ' + id);
        return row.owner;
    });
    const miami = mappings('miami');
    const shapes = [1,5,43].map((id, i) => {
        const row = miami.byId.get(id);
        requireCondition(row && row.owner === [0,6,11][i] && row.type === 2 &&
            row.levels === 5 && row.outer === 0, 'Unexpected Miami shape mapping ' + id);
        const entries = miami.lookup.slice(row.offset, row.offset + row.length);
        requireCondition(entries.length === row.length && row.length > 0, 'Truncated Miami lookup');
        // The setter canonicalizes to five positions; the projector reads the same five positions.
        const values = [...new Set(entries.map(x => Math.floor(x * 4 + 0.5)))];
        requireCondition(values.every(v => v >= 0 && v <= 3), 'Miami shape unsupported');
        return {id, owner:row.owner, values};
    });
    const chicagoWidth = fixed('chicago', chicago, 22), miamiWidth = fixed('miami', miami, 22);
    requireCondition([chicagoWidth, miamiWidth].every(w => w > 0 && w < 1), 'Invalid fixed width');
    return {chicago:{shapes:chicagoShapes, gainOwners:gains, width:chicagoWidth},
            miami:{shapeFields:shapes, width:miamiWidth,
                   possibleShapeTuples:shapes.reduce((n,s) => n*s.values.length,1)}};
}

if (require.main === module) {
    const fs = require('node:fs');
    console.log(JSON.stringify(audit(path => fs.readFileSync(path, 'utf8')), null, 2));
}
module.exports = audit;
