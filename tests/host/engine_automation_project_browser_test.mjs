// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import {parseProject} from '../../web/x0x_projects.js';
import {crc32} from '../../web/x0x_breaks.js';

const bytes=new Uint8Array(36452), view=new DataView(bytes.buffer);
for(const [off,val] of [[0,0x50583058],[4,1],[8,bytes.length-32],[16,2],[24,13],[28,0x4f461701]])
  view.setUint32(off,val,true);
bytes[bytes.length-3]=0xe1; // application bytes remain opaque to the browser.
view.setUint32(12,crc32(bytes.subarray(32)),true);
for(const version of [13,14,15,16]){
  view.setUint32(20,version,true);
  const m=parseProject(bytes,bytes.length);
  assert.equal(m.patternVersion,version);
  assert.equal(m.partOffMask,1);
  assert.equal(bytes[bytes.length-3],0xe1);
}
view.setUint32(20,17,true);
assert.throws(()=>parseProject(bytes),e=>e.code==='version');
view.setUint32(20,16,true);
bytes[bytes.length-3]^=1;
assert.throws(()=>parseProject(bytes),/CRC/);
console.log('PASS current browser accepts version 16 and retains opaque payload, legacy versions, and version/CRC rejection');
