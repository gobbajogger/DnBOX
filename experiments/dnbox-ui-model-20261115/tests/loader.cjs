'use strict';
// Independent extension of the permitted inert-DOM loader. Executes exact source;
// canvases and MessageChannels are real Node objects, not a browser/iframe sandbox.
const fs=require('fs'),vm=require('vm'),crypto=require('crypto');
const {createCanvas,GlobalFonts,Image}=require('@napi-rs/canvas');
const {MessageChannel}=require('worker_threads');const frameHooks={};
async function loadDocument(html,options={}){
 const canvas=createCanvas(240,240),els=new Map(),all=[],events={},timers=new Map(),frames=[];let nextTimer=0,context;
 class El{
  constructor(tag='div',id=''){this.tagName=tag.toUpperCase();this.id=id;this.children=[];this.dataset={};this.attrs={};this.style={};this.value='0';this.hidden=false;this.textContent='';this.events={};this.classList={add(){},remove(){},toggle(){}};all.push(this);if(id)els.set(id,this);}
  set innerHTML(v){this._innerHTML=v;this.children=[];for(const m of v.matchAll(/<(button|input|option|select|p|div|a)\b([^>]*)>/g)){const e=new El(m[1]);for(const a of m[2].matchAll(/([\w-]+)="([^"]*)"/g))e.setAttribute(a[1],a[2]);this.children.push(e);}}
  get innerHTML(){return this._innerHTML||'';}
  getContext(){this.canvas??=this.id==='lcd'?canvas:createCanvas(this.id==='large'?720:240,this.id==='large'?720:240);if(!this.context){const raw=this.canvas.getContext('2d');this.context=new Proxy(raw,{get:(t,k)=>{if(k==='fillRect')return (x,y,w,h)=>{if(this.id==='lcd'&&x===0&&y===0&&w>=240&&h>=240)this.drawn=[];return t.fillRect(x,y,w,h);};if(k==='fillText')return (text,x,y,...a)=>{(this.drawn??=[]).push({text:String(text),x,y,font:t.font,align:t.textAlign});return t.fillText(text,x,y,...a);};if(k==='drawImage')return (im,...a)=>t.drawImage(im?.canvas??im,...a);const v=t[k];return typeof v==='function'?v.bind(t):v;},set(t,k,v){t[k]=v;return true;}});}return this.context;}
  setAttribute(k,v){this.attrs[k]=v;if(k==='id'){this.id=v;els.set(v,this);}if(k==='value')this.value=v;if(k.startsWith('data-'))this.dataset[k.slice(5).replace(/-([a-z])/g,(_,c)=>c.toUpperCase())]=v;}
  getAttribute(k){return this.attrs[k]??null;}
  addEventListener(k,f){(this.events[k]??=[]).push(f);}
  append(...es){this.children.push(...es);for(const e of es)if(e.tagName==='IFRAME'&&e.srcdoc){Promise.resolve().then(async()=>{await frameHooks.beforeLoad?.(e);const child=await loadDocument(e.srcdoc,{parent:context});e.child=child;frames.push(e);e.contentWindow={postMessage(data,origin,ports){for(const fn of child.events.message??[])fn({source:context,data,ports});}};for(const fn of e.events.load??[])fn({target:e});}).catch(e=>{options.errors?.push(String(e));});}}
  appendChild(e){this.append(e);return e;}after(){}replaceChildren(...es){this.children=es;}remove(){this.removed=true;}focus(){}closest(){return null;}scrollIntoView(){}setPointerCapture(){}
  getBoundingClientRect(){return{x:0,y:0,left:0,top:0,width:240,height:240};}
  querySelectorAll(s){return query(s,this.children.flatMap(e=>[e,...e.children]));}querySelector(s){return this.querySelectorAll(s)[0]??null;}
  click(){this.onclick?.({target:this,preventDefault(){}});for(const f of this.events.click||[])f({target:this,preventDefault(){}});}
 }
 function query(s,items=all){if(s.includes(','))return [...new Set(s.split(',').flatMap(a=>query(a.trim(),items)))];if(s.includes(' '))return query(s.split(' ').at(-1),items);const id=s.match(/^#([\w-]+)/);if(id)return els.has(id[1])?[els.get(id[1])]:[];const a=s.match(/\[([\w-]+)(?:=["']?([^\]"']+)["']?)?\]/);if(a)return items.filter(e=>e.getAttribute(a[1])!==null&&(a[2]===undefined||e.getAttribute(a[1])===a[2]));return items.filter(e=>e.tagName.toLowerCase()===s);}
 for(const m of html.matchAll(/\bid="([^"]+)"/g))new El(m[1]==='lcd'||m[1]==='large'?'canvas':'div',m[1]);
 for(const a of ['tab','event','arrow'])for(const m of html.matchAll(new RegExp('<button[^>]*data-'+a+'="([^\\"]+)"','g'))){const e=new El('button');e.setAttribute('data-'+a,m[1]);}
 for(const m of html.matchAll(/@font-face\{font-family:Barlow;font-weight:(\d+);src:url\(data:font\/\w+;base64,([^)]*)\)/g))GlobalFonts.register(Buffer.from(m[2],'base64'),'Barlow');
 const document={body:new El('body'),hidden:false,getElementById:id=>els.get(id)??null,createElement:tag=>new El(tag),querySelectorAll:s=>query(s),querySelector:s=>query(s)[0]??null,fonts:{ready:Promise.resolve(),load:()=>Promise.resolve()},addEventListener(k,f){(events['document:'+k]??=[]).push(f);}};
 const schedule=(f,ms)=>{timers.set(++nextTimer,{f,ms});return nextTimer;};
 context={console,document,Image,structuredClone,performance,Date,Math,Map,Set,Uint8Array,Uint16Array,TextDecoder,URLSearchParams,location:{search:''},MessageChannel,crypto:{randomUUID:crypto.randomUUID},atob:s=>Buffer.from(s,'base64').toString('binary'),btoa:s=>Buffer.from(s,'binary').toString('base64'),setTimeout:schedule,setInterval:schedule,clearTimeout:id=>timers.delete(id),clearInterval:id=>timers.delete(id),requestAnimationFrame:()=>0,addEventListener:(k,f)=>(events[k]??=[]).push(f)};
 context.window=context;context.globalThis=context;context.parent=options.parent??context;vm.createContext(context);
 const scripts=[...html.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/g)].map(m=>m[1]);for(let i=0;i<scripts.length;i++)vm.runInContext(scripts[i],context,{filename:(options.parent?'settings-child':'instrument')+'-'+i+'.js'});
 await new Promise(r=>setImmediate(r));return{p:context.proposal,ctx:context,canvas,els,timers,vm,events,frames};
}
module.exports={setFrameHooks(h){Object.assign(frameHooks,h);},async load(){const inputPath=process.env.CANDIDATE_PATH||'whole-ui-successor/workbench.html',html=fs.readFileSync(inputPath,'utf8'),sha=crypto.createHash('sha256').update(html).digest('hex');if(process.env.EXPECTED_SHA&&sha!==process.env.EXPECTED_SHA)throw Error('Input SHA mismatch '+sha);const x=await loadDocument(html,{errors:[]});x.loadedInput={path:inputPath,sha256:sha};for(let i=0;i<100&&!x.ctx.__ready;i++)await new Promise(r=>setTimeout(r,20));if(!x.ctx.__ready)throw Error('Candidate not ready');return x;},loadDocument};
