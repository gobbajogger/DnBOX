// SPDX-License-Identifier: MIT
// Adapted from gobbajogger/X0X37-for-Loopy-Pro web/dnbox/controls.js
// commit 431d0db82cbadca761fb32c6548ca4765875324f, blob bcbd15eeef70a7688a6ee340622719864d69e49f.
// Copyright (c) 2026 Joakim Eriksson. See LICENSE.
// Input ownership only: no firmware model, voices, MIDI-note inference or device authority.
export function heldRouter(send) {
  if (typeof send !== 'function') throw new TypeError('send required');
  const held = new Map(); // unique source token -> original {kind,id}
  const owners = new Map(); // kind:id -> independent source count
  const listeners = new Set();
  const slot = (kind,id) => kind+':'+id;
  const valid = (kind,id) => Number.isInteger(id) && id>=0 &&
    ((kind==='physical' && id<37) || (kind==='pad' && id<16));
  const notify = () => { for(const listener of listeners) listener(); };
  const release = token => {
    const item=held.get(token);
    if(!item) return false;
    held.delete(token);
    const key=slot(item.kind,item.id), count=owners.get(key);
    if(count===1) owners.delete(key); else owners.set(key,count-1);
    notify();
    if(count===1) send(item.kind,item.id,0);
    return true;
  };
  return Object.freeze({
    press(token,kind,id) {
      if(!valid(kind,id)) throw new RangeError('invalid native37 control');
      if(typeof token!=='string' || !token) throw new TypeError('source token required');
      if(held.has(token)) return false;
      const key=slot(kind,id), count=owners.get(key)||0;
      held.set(token,{kind,id});owners.set(key,count+1);notify();
      if(count===0) send(kind,id,kind==='physical'?100:1);
      return true;
    },
    release,
    releaseAll(){for(const token of [...held.keys()]) release(token);},
    countFor(kind,id){return owners.get(slot(kind,id))||0;},
    has(token){return held.has(token);},
    subscribe(listener){listeners.add(listener);return ()=>listeners.delete(listener);},
    get count(){return held.size;}
  });
}
let mountSerial=0;
/** getId is sampled ONLY for a fresh activation; releases retain their original target.
 * Call bindCancellation once for the owning surface. reset/destroy here affect this mount only.
 */
export function mountMomentary(button,{router,kind,getId}) {
  const prefix='native37-mount:'+ ++mountSerial + ':';
  const tokens=new Set(), bindings=[];
  const pToken=id=>prefix+'pointer:'+id;
  const kToken=key=>prefix+'keyboard:'+key;
  const sync=()=>button.setAttribute('aria-pressed',String([...tokens].some(t=>router.has(t))));
  const on=(name,fn)=>{button.addEventListener(name,fn);bindings.push([name,fn]);};
  const press=(token,event)=>{
    if(button.disabled || router.has(token)) return;
    // Track before press so router notification sees this mount's active ownership.
    tokens.add(token);
    try {router.press(token,kind,getId());} catch(error){if(!router.has(token))tokens.delete(token);sync();throw error;}
    event.preventDefault();sync();
  };
  const release=token=>{router.release(token);tokens.delete(token);sync();};
  const reset=()=>{for(const token of [...tokens])release(token);};
  button.type='button';sync();
  const unsubscribe=router.subscribe(sync);
  on('pointerdown',e=>{
    if(e.button!==0 || !Number.isInteger(e.pointerId))return;
    const token=pToken(e.pointerId);press(token,e);
    if(router.has(token)){try{button.setPointerCapture(e.pointerId);}catch{/* release/cancel/global reset remain active */}}
  });
  for(const name of ['pointerup','pointercancel','lostpointercapture'])on(name,e=>release(pToken(e.pointerId)));
  on('keydown',e=>{if((e.key===' '||e.key==='Enter')&&!e.repeat)press(kToken(e.key),e);});
  on('keyup',e=>{if(e.key===' '||e.key==='Enter'){e.preventDefault();release(kToken(e.key));}});
  on('blur',()=>{release(kToken(' '));release(kToken('Enter'));});
  let destroyed=false;
  return Object.freeze({reset,destroy(){if(destroyed)return;destroyed=true;reset();unsubscribe();for(const [name,fn]of bindings)button.removeEventListener(name,fn);}});
}
/** Window blur or document hidden releases every source. Visibility regain never replays presses. */
export function bindCancellation(router,{win,doc}) {
  const reset=()=>router.releaseAll();
  const visibility=()=>{if(doc.hidden)reset();};
  win.addEventListener('blur',reset);doc.addEventListener('visibilitychange',visibility);
  let destroyed=false;
  return Object.freeze({reset,destroy(){if(destroyed)return;destroyed=true;reset();win.removeEventListener('blur',reset);doc.removeEventListener('visibilitychange',visibility);}});
}
