#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-only
// Reconstruct exact public test resources; no native/runtime admission implied.
const fs=require('fs'),path=require('path'),crypto=require('crypto');
function rebuild(upstream,output,codec=path.resolve(__dirname,'../tests/fixtures/synthone-morph-codec')) {
 const a=require(path.join(codec,'compact_analysis.js'));
 const m=JSON.parse(fs.readFileSync(path.join(codec,'input_manifest.json'))),c=JSON.parse(fs.readFileSync(path.join(codec,'fold_config.json')));
 const rows=m.source_json_manifest.map(x=>({returned_blob:x.blob_sha,content:fs.readFileSync(path.join(upstream,x.path),'utf8')}));
 const input=a.reconstructCanonical(rows,m),enc=a.buildMorphContainer(input,c),pack=Buffer.from(enc.bytes);
 const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
 if(hash(pack)!==m.expected_container_sha256)throw Error('compact identity');
 const raw=Buffer.alloc(52*4097*4);input.logical.forEach((r,i)=>r.words.forEach((w,j)=>raw.writeUInt32LE(w,i*4097*4+j*4)));
 if(hash(raw)!==m.raw52_sha256)throw Error('raw identity');
 // All source/output identities are checked before creating any output.
 fs.mkdirSync(output,{recursive:false});
 fs.writeFileSync(path.join(output,'compact.bin'),pack,{flag:'wx'});
 fs.writeFileSync(path.join(output,'public-morphs.f32'),raw,{flag:'wx'});
 return {scope:'exact public test-input reconstruction, not runtime or target qualification',compact:{bytes:pack.length,sha256:hash(pack)},raw:{bytes:raw.length,sha256:hash(raw)}};
}
module.exports={rebuild};
if(require.main===module){if(process.argv.length!==4)throw Error('usage: rebuild-synthone-morph-inputs.cjs UPSTREAM_ROOT FRESH_OUTPUT');console.log(JSON.stringify(rebuild(path.resolve(process.argv[2]),path.resolve(process.argv[3]))));}
