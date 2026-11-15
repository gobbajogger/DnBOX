// SPDX-License-Identifier: GPL-3.0-only
// Pure analysis-memory lossless morph prototype. No C/native/runtime admission.
const S1CompactAnalysis = (() => {

function hashBinary(s,kind="sha256"){
 const bs=s;const a=[];for(let i=0;i<bs.length;i++)a.push(bs.charCodeAt(i));const bit=a.length*8;a.push(128);while(a.length%64!==56)a.push(0);for(let i=7;i>=0;i--)a.push(i>=4?0:(bit>>>i*8)&255);
 const rot=(x,n)=>(x>>>n)|(x<<(32-n));
 let h=kind==="sha1"?[0x67452301,0xefcdab89,0x98badcfe,0x10325476,0xc3d2e1f0]:[0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19];
 const K=[0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2];
 for(let offset=0;offset<a.length;offset+=64){
 let w=[];for(let i=0;i<16;i++)w[i]=(a[offset+i*4]<<24)|(a[offset+i*4+1]<<16)|(a[offset+i*4+2]<<8)|a[offset+i*4+3];
 if(kind==="sha1"){
 for(let i=16;i<80;i++){let x=w[i-3]^w[i-8]^w[i-14]^w[i-16];w[i]=(x<<1)|(x>>>31);}
 let [A,B,C,D,E]=h;
 for(let i=0;i<80;i++){const f=i<20?(B&C)|(~B&D):i<40?B^C^D:i<60?(B&C)|(B&D)|(C&D):B^C^D;const k=i<20?0x5a827999:i<40?0x6ed9eba1:i<60?0x8f1bbcdc:0xca62c1d6;const temp=(((A<<5)|(A>>>27))+f+E+k+w[i])|0;E=D;D=C;C=(B<<30)|(B>>>2);B=A;A=temp;}[A,B,C,D,E].forEach((v,i)=>h[i]=(h[i]+v)|0);
 }else{
 for(let i=16;i<64;i++){const x=w[i-15],y=w[i-2];w[i]=((rot(x,7)^rot(x,18)^(x>>>3))+w[i-16]+(rot(y,17)^rot(y,19)^(y>>>10))+w[i-7])|0;}
 let[A,B,C,D,E,F,G,H]=h;
 for(let i=0;i<64;i++){const t1=(H+(rot(E,6)^rot(E,11)^rot(E,25))+((E&F)^(~E&G))+K[i]+w[i])|0;const t2=((rot(A,2)^rot(A,13)^rot(A,22))+((A&B)^(A&C)^(B&C)))|0;H=G;G=F;F=E;E=(D+t1)|0;D=C;C=B;B=A;A=(t1+t2)|0;}[A,B,C,D,E,F,G,H].forEach((v,i)=>h[i]=(h[i]+v)|0);
 }}return h.map(v=>(v>>>0).toString(16).padStart(8,"0")).join("");
}

function hashUtf8(s,kind="sha256"){ return hashBinary(unescape(encodeURIComponent(s)),kind); }

function blockPlan(a){
 const n=a.length,bw=x=>x?32-Math.clz32(x>>>0):0,zz=x=>((x<<1)^(x>>31))>>>0;
 const opts=[{mode:0,width:32,anchors:[],codes:a.slice(),bytes:2+4*n}];
 const min=Math.min(...a),ref=a[0],add=(mode,anchors,codes,dict)=>{
 const width=bw(codes.reduce((m,x)=>Math.max(m,x),0)),bytes=2+4*anchors.length+Math.ceil(n*width/8)+(dict?1+4*dict.length:0);
 opts.push({mode,width,anchors,codes,dict,bytes});};
 add(1,[min],a.map(w=>(w-min)>>>0));
 const step=n>1?Math.trunc(((a[n-1]-ref)|0)/(n-1))>>>0:0;
 add(3,[ref,step],a.map((w,i)=>zz((w-ref-Math.imul(step,i))|0)));
 const st=n>1?(a[1]-ref)>>>0:0,cv=n>2?(a[2]-2*a[1]+ref)>>>0:0;
 add(4,[ref,st,cv],a.map((w,i)=>zz((w-ref-Math.imul(st,i)-Math.imul(cv,i*(i-1)/2))|0)));
 opts.sort((a,b)=>a.bytes-b.bytes||a.mode-b.mode);return opts[0];
}

function encodeCanonical(input,config,blockPlan,B=64){
 const N=4096,H=2048,mask=4095,rows=[];
 for(let r=0;r<50;r++){
 const src=input.physical[r],cf=config[r],mode=cf.mode,rotation=cf.rotation,a=cf.a,b=cf.b,bc=mode==="quarter"?1025:mode==="half"?2049:4096;
 const base=Array.from({length:bc},(_,i)=>src.words[(i+rotation)&mask]),patches=[];
 for(let i=0;i<N;i++){
 const j=(i-rotation+N)&mask;let rep=j,sign=0;
 const ts=mode==="raw"?[[j,0]]:mode==="half"?[[j,0],[(-j)&mask,a]]:[[j,0],[(-j)&mask,a],[(j+H)&mask,b],[(-j+H)&mask,a^b]];
 for(const [p,s] of ts)if(p<rep){rep=p;sign=s;}
 if(((base[rep]^(sign?0x80000000:0))>>>0)!==src.words[i])patches.push([i,src.words[i]]);
 }
 const blocks=[];for(let i=0;i<base.length;i+=B)blocks.push(blockPlan(base.slice(i,i+B)));
 const runs=[];for(let i=0;i<N;){let j=i+1;while(j<N&&src.words[j]===src.words[i])j++;runs.push([j,src.words[i]]);i=j;}
 const cost=blocks.reduce((n,p)=>n+p.bytes+4,0)+patches.length*6;
 rows.push(runs.length*6<cost?{name:src.name,rle:runs}:{name:src.name,mode,rotation,a,b,base_count:bc,patches,blocks});
 }
 const blockCount=rows.reduce((n,r)=>n+(r.blocks?.length||0),0),payloadSize=rows.reduce((n,r)=>n+(r.blocks?.reduce((n,p)=>n+p.bytes,0)||0),0),aux=rows.reduce((n,r)=>n+6*(r.rle?.length||r.patches.length),0);
 const descOff=64,mapOff=1264,bandsOff=1316,indexOff=1368,blocksOff=indexOff+4*(blockCount+1),patchOff=(blocksOff+payloadSize+3)&~3,total=(patchOff+aux+3)&~3;
 const bytes=new Uint8Array(total),dv=new DataView(bytes.buffer),put=(p,v)=>dv.setUint32(p,v>>>0,true);let bi=0,cursor=blocksOff;
 for(let r=0;r<50;r++){
 const q=rows[r],d=descOff+r*24;
 if(q.rle){put(d,3);put(d+8,N);continue;}
 put(d,q.mode==="quarter"?2:q.mode==="half"?1:0);put(d+4,q.rotation|(q.a<<12)|(q.b<<13));put(d+8,q.base_count);put(d+12,bi);
 for(const p of q.blocks){
 const start=cursor;put(indexOff+4*bi++,start);bytes[cursor++]=p.mode;bytes[cursor++]=p.width;
 for(const x of p.anchors){put(cursor,x);cursor+=4;}if(p.dict)throw Error("unsupported unused dictionary");
 if(p.mode===0)for(const x of p.codes){put(cursor,x);cursor+=4;}
 else{
 const off=cursor;cursor+=Math.ceil(p.codes.length*p.width/8);
 for(let i=0;i<p.codes.length;i++)for(let j=0;j<p.width;j++)if((p.codes[i]>>>j)&1)bytes[off+((i*p.width+j)>>>3)]|=1<<((i*p.width+j)&7);
 }
 if(cursor-start!==p.bytes)throw Error("block size");
 }
 }
 put(indexOff+4*bi,cursor);if(bi!==blockCount||((cursor+3)&~3)!==patchOff)throw Error("layout");
 let pc=patchOff;
 for(let r=0;r<50;r++){
 const q=rows[r],d=descOff+r*24;
 if(q.rle){put(d+12,pc);put(d+16,q.rle.length);for(const [end,w] of q.rle){dv.setUint16(pc,end,true);put(pc+2,w);pc+=6;}}
 else{put(d+16,pc);put(d+20,q.patches.length);for(const [i,w] of q.patches){dv.setUint16(pc,i,true);put(pc+2,w);pc+=6;}}
 }
 if(((pc+3)&~3)!==total)throw Error("total size");
 input.mapping.forEach((v,i)=>bytes[mapOff+i]=v);input.bands.forEach((v,i)=>dv.setFloat32(bandsOff+4*i,v,true));
 [0x314c3153,1,50,52,4097,B,descOff,mapOff,bandsOff,indexOff,blockCount,blocksOff,patchOff,total,24,0].forEach((v,i)=>put(4*i,v));
 return {bytes:Array.from(bytes),block_count:blockCount,payload_size:payloadSize,aux_size:aux,rows:rows.map(r=>({name:r.name,mode:r.rle?"rle":r.mode,patches:r.patches?.length||0,rle:r.rle?.length||0}))};
}

function admitCompact(bytes){
 try{
 if(!(bytes instanceof Uint8Array)||bytes.length<64)return null;
 const d=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),u32=p=>{if(p<0||p+4>bytes.length)throw Error("u32 bounds");return d.getUint32(p,true);},u16=p=>{if(p<0||p+2>bytes.length)throw Error("u16 bounds");return d.getUint16(p,true);};
 const magic=u32(0),version=u32(4),B=u32(20),desc=u32(24),map=u32(28),bands=u32(32),idx=u32(36),bc=u32(40),data=u32(44),patch=u32(48),total=u32(52);
 if(magic!==0x314c3153||version!==1||u32(8)!==50||u32(12)!==52||u32(16)!==4097||B!==64||desc!==64||map!==1264||bands!==1316||idx!==1368||bc>3200||data!==idx+4*(bc+1)||data>bytes.length||patch<data||patch>bytes.length||total!==bytes.length||u32(56)!==24||u32(60)!==0)return null;
 const mapping=[...Array(48).keys(),44,45,48,49];for(let i=0;i<52;i++)if(bytes[map+i]!==mapping[i])return null;
 for(let i=0;i<13;i++){const f=d.getFloat32(bands+4*i,true);if(!Number.isFinite(f)||f<0||(i&&f<=d.getFloat32(bands+4*(i-1),true)))return null;}
 if(u32(idx)!==data||u32(idx+bc*4)>patch||((u32(idx+bc*4)+3)&~3)!==patch)return null;
 let runningBlock=0,runningPatch=patch;
 for(let r=0;r<50;r++){
 const q=desc+24*r,mode=u32(q),flags=u32(q+4),n=u32(q+8),first=u32(q+12),off=u32(q+16),cnt=u32(q+20);
 if(mode>3||flags>>>14)return null;
 if(mode===3){
 if(flags||n!==4096||first!==runningPatch||off<1||off>4096||cnt||first+6*off>bytes.length)return null;
 let prev=0;for(let i=0;i<off;i++){const end=u16(first+6*i);if(end<=prev||end>4096)return null;prev=end;}if(prev!==4096)return null;runningPatch+=6*off;continue;
 }
 const want=mode===0?4096:mode===1?2049:1025;if(n!==want||first!==runningBlock||off!==runningPatch||cnt>4096||off+6*cnt>bytes.length)return null;
 if(mode===0&&flags)return null;if(mode===1&&((flags>>>13)&1))return null;
 const nb=Math.ceil(n/B);if(first+nb>bc)return null;
 for(let j=0;j<nb;j++){
 const p=u32(idx+4*(first+j)),end=u32(idx+4*(first+j+1)),count=Math.min(B,n-j*B);
 if(p<data||end<=p||end>patch||p+2>end)return null;
 const bm=bytes[p],width=bytes[p+1],anchors=bm===0?0:bm===1?1:bm===3?2:bm===4?3:-1;
 if(anchors<0||width>32||(bm===0&&width!==32))return null;
 const length=2+4*anchors+(bm===0?4*count:Math.ceil(count*width/8));if(end-p!==length)return null;
 }
 let prev=-1;for(let j=0;j<cnt;j++){const sample=u16(off+6*j);if(sample<=prev||sample>=4096)return null;prev=sample;}
 runningBlock+=nb;runningPatch+=6*cnt;
 }
 if(runningBlock!==bc||((runningPatch+3)&~3)!==total)return null;
 for(let p=runningPatch;p<total;p++)if(bytes[p])return null;
 function physicalWord(r,s){
 if(s===4096)return 0;
 const q=desc+24*r,mode=u32(q),flags=u32(q+4),n=u32(q+8),first=u32(q+12),off=u32(q+16),cnt=u32(q+20);
 if(mode===3){let lo=0,hi=off;while(lo<hi){const mid=(lo+hi)>>>1;if(s<u16(first+6*mid))hi=mid;else lo=mid+1;}if(lo===off)throw Error("RLE end");return u32(first+6*lo+2);}
 let lo=0,hi=cnt;while(lo<hi){const mid=(lo+hi)>>>1;if(u16(off+6*mid)<s)lo=mid+1;else hi=mid;}if(lo<cnt&&u16(off+6*lo)===s)return u32(off+6*lo+2);
 let j=(s-(flags&4095)+4096)&4095,rep=j,sign=0,a=(flags>>>12)&1,b=(flags>>>13)&1;
 const transforms=mode===0?[[j,0]]:mode===1?[[j,0],[(-j)&4095,a]]:[[j,0],[(-j)&4095,a],[(j+2048)&4095,b],[(-j+2048)&4095,a^b]];
 for(const [p,z] of transforms)if(p<rep){rep=p;sign=z;}if(rep>=n)throw Error("sample map");
 const block=first+Math.floor(rep/B),p=u32(idx+4*block),bm=bytes[p],width=bytes[p+1],i=rep%B,anchors=bm===0?0:bm===1?1:bm===3?2:3;
 let value;
 if(bm===0)value=u32(p+2+4*i);
 else{
 let code=0,packed=p+2+4*anchors;for(let k=0;k<width;k++){const bit=i*width+k;if((bytes[packed+(bit>>>3)]>>>(bit&7))&1)code=(code|(1<<k))>>>0;}
 const base=u32(p+2);
 if(bm===1)value=(base+code)>>>0;
 else{const residual=(code>>>1)^-(code&1),step=u32(p+6),curv=bm===4?u32(p+10):0;value=(base+Math.imul(step,i)+Math.imul(curv,i*(i-1)/2)+residual)>>>0;}
 }
 return (value^(sign?0x80000000:0))>>>0;
 }
 const temp=new DataView(new ArrayBuffer(4));
 for(let r=0;r<50;r++)for(let s=0;s<4097;s++){const w=physicalWord(r,s);temp.setUint32(0,w,true);const f=temp.getFloat32(0,true);if(!Number.isFinite(f)||f< -2||f>2||(s===4096&&w!==0))return null;}
 return {read(row,sample,out){
 if(!Number.isInteger(row)||row<0||row>=52||!Number.isInteger(sample)||sample<0||sample>=4097||!out||typeof out!=="object")return false;
 try{const v=physicalWord(mapping[row],sample);out.word=v;return true;}catch{return false;}
 }};
 }catch{return null;}
}

function reconstructCanonical(sourceRows,manifest){
 const expect=manifest.source_json_manifest;
 for(const e of sourceRows){
 const m=expect.find(x=>x.blob_sha===e.returned_blob);
 if(!m||e.content.length!==m.size||hashUtf8(e.content)!==m.sha256||hashBinary("blob "+m.size+"\0"+e.content,"sha1")!==m.blob_sha)throw Error("public JSON identity");
 }
 for(const meta of [manifest.order_metadata,manifest.frequency_metadata]){
 if(meta.content.length!==meta.size||hashUtf8(meta.content)!==meta.sha256||hashBinary("blob "+meta.size+"\0"+meta.content,"sha1")!==meta.blob_sha)throw Error("metadata identity");
 }
 const order=JSON.parse(manifest.order_metadata.content),bands=JSON.parse(manifest.frequency_metadata.content).content,dv=new DataView(new ArrayBuffer(4));
 if(order.length!==52||bands.length!==13)throw Error("topology");
 const logical=order.map((name,i)=>{
 const m=expect.find(e=>e.path.endsWith("/"+name+".json")),src=sourceRows.find(e=>e.returned_blob===m?.blob_sha);
 if(!m||!src)throw Error("missing public row "+name);
 const j=JSON.parse(src.content);if(j.content.length!==4096||j.phase!==0||j.type!==(i<4?[1,2,0,3][i]:10))throw Error("source row schema");
 const words=j.content.map(x=>{if(!Number.isFinite(x)||x< -2||x>2)throw Error("sample domain");dv.setFloat32(0,x,true);return dv.getUint32(0,true);});words.push(0);
 return {name,blob:m.blob_sha,words};
 });
 const physical=[...logical.slice(0,48),...logical.slice(50)],mapping=[...Array(48).keys(),44,45,48,49];
 const raw=wordString(logical),packed=wordString(physical);
 if(hashBinary(raw)!==manifest.raw52_sha256||raw.length!==852176||hashBinary(packed)!==manifest.physical50_sha256||packed.length!==819400)throw Error("canonical corpus hash");
 return {logical,physical,mapping,bands};
}

function wordString(rows){let out="";for(const r of rows)for(const w of r.words)out+=String.fromCharCode(w&255,(w>>>8)&255,(w>>>16)&255,w>>>24);return out;}

function byteString(bytes){let out="";for(const b of bytes)out+=String.fromCharCode(b);return out;}

function buildMorphContainer(input,config){
 if(!input||!Array.isArray(input.physical)||input.physical.length!==50||!Array.isArray(input.logical)||input.logical.length!==52||!Array.isArray(input.bands)||input.bands.length!==13||!Array.isArray(input.mapping)||input.mapping.length!==52||!Array.isArray(config)||config.length!==50)throw Error("encoder topology");
 const expected=[...Array(48).keys(),44,45,48,49];
 for(let i=0;i<52;i++)if(input.mapping[i]!==expected[i])throw Error("logical map");
 const dv=new DataView(new ArrayBuffer(4));
 for(let r=0;r<50;r++){
 const q=config[r],row=input.physical[r];
 if(!q||!row||q.name!==row.name||!["raw","half","quarter"].includes(q.mode)||!Number.isInteger(q.rotation)||q.rotation<0||q.rotation>4095||![0,1].includes(q.a)||![0,1].includes(q.b)||(q.mode==="raw"&&(q.rotation||q.a||q.b))||(q.mode==="half"&&q.b)||!Array.isArray(row.words)||row.words.length!==4097||row.words[4096]!==0)throw Error("encoder row contract");
 for(let i=0;i<4097;i++){const w=row.words[i];if(!Number.isInteger(w)||w<0||w>0xffffffff)throw Error("word type");dv.setUint32(0,w,true);const f=dv.getFloat32(0,true);if(!Number.isFinite(f)||f< -2||f>2)throw Error("word domain");}
 }
 for(let r=0;r<52;r++){
 const row=input.logical[r],physical=input.physical[expected[r]];
 if(!row||!Array.isArray(row.words)||row.words.length!==4097)throw Error("logical row contract");
 for(let i=0;i<4097;i++)if(!Number.isInteger(row.words[i])||row.words[i]<0||row.words[i]>0xffffffff||row.words[i]!==physical.words[i])throw Error("logical physical word mismatch");
 }
 let previous=-Infinity;
 for(let i=0;i<13;i++){
 const v=input.bands[i];if(!Number.isFinite(v)||v<0||(i&&v<=input.bands[i-1]))throw Error("bands");
 dv.setFloat32(0,v,true);const rounded=dv.getFloat32(0,true);
 if(!Number.isFinite(rounded)||rounded<0||(i&&rounded<=previous))throw Error("binary32 bands");previous=rounded;
 }
 const result=encodeCanonical(input,config,blockPlan,64);
 if(!admitCompact(Uint8Array.from(result.bytes)))throw Error("encoded container rejected by v1 contract");
 return result;
}

function compareAllWords(reader,input){
 if(!reader)throw Error("valid reader rejected");let checks=0;
 for(let r=0;r<52;r++)for(let s=0;s<4097;s++){const out={word:0xdeadbeef};if(!reader.read(r,s,out)||out.word!==input.logical[r].words[s])throw Error("word mismatch "+r+":"+s);checks++;}
 return checks;
}

function negativeChecks(original){
 const dv=new DataView(original.buffer,original.byteOffset,original.length),reader=admitCompact(original);if(!reader)throw Error("negative fixture invalid");let rejected=0,reads=0;
 const reject=(name,fn,custom)=>{const b=custom||original.slice(),v=new DataView(b.buffer,b.byteOffset,b.byteLength);fn?.(b,v);if(admitCompact(b)!==null)throw Error("malformed admitted "+name);rejected++;};
 for(const n of [0,1,63,64,100,1367,6171,original.length-1])reject("truncated",null,original.slice(0,n));
 for(let i=0;i<16;i++)reject("header",(b,v)=>v.setUint32(4*i,dv.getUint32(4*i,true)^0x80000000,true));
 for(let r=0;r<52;r++)reject("map",b=>b[1264+r]=255);
 for(let i=0;i<13;i++)reject("band",(b,v)=>v.setUint32(1316+4*i,0x7fc00001,true));
 let firstRle=-1;
 for(let r=0;r<50;r++){
 const q=64+24*r,mode=dv.getUint32(q,true);
 for(let f=0;f<6;f++)reject("descriptor",(b,v)=>{const p=q+4*f;v.setUint32(p,f===0?4:f===1?0x40000000:dv.getUint32(p,true)+1,true);});
 if(mode===3){
 if(firstRle<0)firstRle=r;const p=dv.getUint32(q+12,true);
 reject("RLE end",(b,v)=>v.setUint16(p,0,true));reject("RLE NaN",(b,v)=>v.setUint32(p+2,0x7fc00000,true));
 }else{
 const p=dv.getUint32(q+16,true),n=dv.getUint32(q+20,true);
 for(let j=0;j<n;j++){reject("patch index",(b,v)=>v.setUint16(p+6*j,4096,true));reject("patch NaN",(b,v)=>v.setUint32(p+6*j+2,0x7fc00000,true));}
 }
 }
 const index=dv.getUint32(36,true),bc=dv.getUint32(40,true);
 for(let j=0;j<bc;j++){const p=dv.getUint32(index+4*j,true);reject("block mode",b=>b[p]=255);reject("block width",b=>b[p+1]=33);reject("block index",(b,v)=>v.setUint32(index+4*j,p+1,true));}
 if(dv.getUint32(52,true)%4===0)reject("padding",b=>b[b.length-1]=1);
 for(const r of [-1,52,NaN,Infinity,1.1,"1"]){const out={word:0xdeadbeef};if(reader.read(r,0,out)||out.word!==0xdeadbeef)throw Error("bad read row");reads++;}
 for(let r=0;r<52;r++)for(const s of [-1,4097,NaN,Infinity,0.5,"0"]){const out={word:0xdeadbeef};if(reader.read(r,s,out)||out.word!==0xdeadbeef)throw Error("bad read sample");reads++;}
 for(const out of [null,undefined,0,"x"]){if(reader.read(0,0,out))throw Error("bad read output");reads++;}
 let finiteControl=false;
 if(firstRle>=0){
 const b=original.slice(),v=new DataView(b.buffer),p=v.getUint32(64+24*firstRle+12,true);v.setUint32(p+2,v.getUint32(p+2,true)^1,true);
 const changed=admitCompact(b);if(!changed)throw Error("finite wrong-word control structurally rejected");
 const logical=firstRle<48?firstRle:firstRle+2,a={word:0},z={word:0};reader.read(logical,0,a);changed.read(logical,0,z);if(a.word===z.word)throw Error("wrong-word control not detected");finiteControl=true;
 }
 return {malformed_rejected:rejected,invalid_reads_output_unchanged:reads,finite_wrong_word_detected:finiteControl,format_is_content_authentication:false};
}

function runSyntheticTests(){
 const dv=new DataView(new ArrayBuffer(4)),physical=[];
 for(let r=0;r<50;r++){
 let seed=12345+r,words=[];
 for(let i=0;i<4096;i++){
 const j=i%64;let w;
 if(r===0)w=0x3f000000;
 else if(r===5)w=[0x3f800000,0xbf800000,0][i%3];
 else if(r%5===1)w=0x3f000000+(i&1);
 else if(r%5===2)w=0x3e000000+3*j;
 else if(r%5===3)w=0x3e000000+3*j+j*(j-1)/2;
 else{seed=(Math.imul(seed,1664525)+1013904223)>>>0;w=((seed&1?0x80000000:0)|0x3e000000|(seed&0x7fffff))>>>0;}
 words.push(w>>>0);
 }words.push(0);physical.push({name:"synthetic_"+r,words});
 }
 const mapping=[...Array(48).keys(),44,45,48,49],logical=mapping.map(i=>physical[i]),input={physical,logical,mapping,bands:[0,1,2,3,4,5,6,7,8,9,10,11,12]},config=physical.map(r=>({name:r.name,mode:"raw",rotation:0,a:0,b:0}));
 const enc=buildMorphContainer(input,config),bytes=Uint8Array.from(enc.bytes),reader=admitCompact(bytes),checks=compareAllWords(reader,input),v=new DataView(bytes.buffer),modes=new Set(),idx=v.getUint32(36,true),n=v.getUint32(40,true);
 for(let i=0;i<n;i++)modes.add(bytes[v.getUint32(idx+4*i,true)]);for(const m of [0,1,3,4])if(!modes.has(m))throw Error("synthetic block mode not reached");
 const cfg=config.map(x=>({...x}));cfg[0].mode="invalid";let bad=false;try{buildMorphContainer(input,cfg);}catch{bad=true;}if(!bad)throw Error("bad fold accepted");
 const mustReject=fn=>{let failed=false;try{fn();}catch{failed=true;}if(!failed)throw Error("invalid encoder input accepted");};
 mustReject(()=>buildMorphContainer({...input,mapping:new Array(52)},config));
 const conflict=logical.map(r=>({...r,words:r.words.slice()}));conflict[48].words[0]=(conflict[48].words[0]^1)>>>0;
 mustReject(()=>buildMorphContainer({...input,logical:conflict},config));
 const overflow=input.bands.slice();overflow[12]=Number.MAX_VALUE;
 mustReject(()=>buildMorphContainer({...input,bands:overflow},config));
 const collapsed=input.bands.slice();collapsed[2]=collapsed[1]+1e-12;
 mustReject(()=>buildMorphContainer({...input,bands:collapsed},config));
 return {word_checks:checks,block_modes:[...modes].sort(),container_bytes:bytes.length,container_sha256:hashBinary(byteString(bytes)),invalid_fold_rejected:true,encoder_input_regressions_rejected:4};
}

function runFullTests(sourceRows,manifest,config){
 if(hashUtf8("")!=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"||hashUtf8("abc")!=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"||hashUtf8("abc","sha1")!=="a9993e364706816aba3e25717850c26c9cd0d89d")throw Error("hash vectors");
 const synthetic=runSyntheticTests(),input=reconstructCanonical(sourceRows,manifest),enc=buildMorphContainer(input,config),bytes=Uint8Array.from(enc.bytes),reader=admitCompact(bytes),sha=hashBinary(byteString(bytes));
 if(sha!==manifest.expected_container_sha256)throw Error("encoded artifact identity");
 const words=compareAllWords(reader,input),negatives=negativeChecks(bytes),after=hashBinary(byteString(bytes));if(after!==sha)throw Error("read mutated input");
 const again=buildMorphContainer(input,config);if(again.bytes.length!==bytes.length||again.bytes.some((v,i)=>v!==bytes[i]))throw Error("encoder not reproducible");
 return {scope:"Pure analysis-memory JS source/word/error proof only; NOT compiled C, PCM, native/resource/CPU/device acceptance",synthetic,full_logical_word_checks:words,morph_container_bytes:bytes.length,morph_container_sha256:sha,negatives,deterministic_encoder:true,input_unchanged:true,raw_sine_additional_bytes:16388,tuning_additional_bytes:512,complete_asset_input_bytes:bytes.length+16388+512,not_run:["C decoder and exact oscillator/scheduled PCM","native decoder code/state/stack/cache/CPU","current combined product occupancy and safe storage placement","independent acceptance"]};
}

return { hashUtf8, hashBinary, reconstructCanonical, buildMorphContainer, admitCompact, runSyntheticTests, runFullTests };

})();
if (typeof module !== 'undefined' && module.exports) module.exports = S1CompactAnalysis;
S1CompactAnalysis;
