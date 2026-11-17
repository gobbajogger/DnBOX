// SPDX-License-Identifier: GPL-3.0-only
const assert=require('assert'),fs=require('fs'),os=require('os'),path=require('path');
const {rebuild}=require('../../tools/rebuild-synthone-morph-inputs.cjs');
if(process.argv.length!==3)throw Error('usage: node synthone_input_rebuild_test.cjs PINNED_UPSTREAM_ROOT');
const temp=fs.mkdtempSync(path.join(os.tmpdir(),'s1-input-rebuild-'));
try{
 const output=path.join(temp,'good'),r=rebuild(path.resolve(process.argv[2]),output);
 assert.deepStrictEqual(r.compact,{bytes:165292,sha256:'a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5'});
 assert.deepStrictEqual(r.raw,{bytes:852176,sha256:'0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'});
 const before=fs.readFileSync(path.join(output,'compact.bin'));
 assert.throws(()=>rebuild(path.resolve(process.argv[2]),output));
 assert.deepStrictEqual(fs.readFileSync(path.join(output,'compact.bin')),before);
 const missing=path.join(temp,'missing-output');assert.throws(()=>rebuild(path.join(temp,'absent-source'),missing));assert(!fs.existsSync(missing));
 console.log('Synth One exact input reconstruction: hashes, fresh-output refusal and absent-input rejection PASS');
}finally{fs.rmSync(temp,{recursive:true,force:true});}
