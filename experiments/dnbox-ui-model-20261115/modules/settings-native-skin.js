/* SPDX-License-Identifier: GPL-3.0-only */
/* Native-family visual proposal over the accepted typed bridge. No new host method. */
(()=>{
 const originalMount=window.mountSharedSettings;
 window.mountSharedSettings=async function(port){
  const api=await originalMount(port),priorRender=render;
  const P={bg:'#050a0d',face:'#101a1f',edge:'#26383d',side:'#070e11',sel:'#253b2d',ink:'#e0ebd6',value:'#dee6de',gray:'#9caaa8',dim:'#809294',line:'#28383d',amber:'#c49b45',teal:'#41ceff'};
  function r(x,y,w,h,c){ctx.fillStyle=c;ctx.fillRect(x,y,w,h);}
  function l(x,y,x2,y2,c,w=1){ctx.strokeStyle=c;ctx.lineWidth=w;ctx.beginPath();ctx.moveTo(x+.5,y+.5);ctx.lineTo(x2+.5,y2+.5);ctx.stroke();}
  function t(v,x,y,size=11,col=P.gray,bold=false,align='left'){ctx.font=(bold?'600':'500')+' '+size+'px Barlow';ctx.textBaseline='top';ctx.textAlign=align;ctx.fillStyle=col;ctx.fillText(String(v),x,y);}
  function slab(x,y,w,h,selected=false){ctx.fillStyle=selected?P.sel:P.face;ctx.beginPath();ctx.roundRect(x,y,w,h,h>=18?4:2);ctx.fill();l(x+4,y,x+w-5,y,P.edge);l(x+4,y+h-1,x+w-5,y+h-1,P.side);}
  // Existing native PERFORM primitives, translated into the shared158px strip.
  function glyph(i,value,applied){const cx=30+i*60,dy=158,col=P.amber;const ln=(x,y,x2,y2,c=col)=>l(x,y+dy,x2,y2+dy,c),rc=(x,y,w,h,c=col)=>r(x,y+dy,w,h,c);
   if(i===0){ln(cx-10,47,cx-4,28);ln(cx-4,28,cx+4,28);ln(cx+4,28,cx+10,47);ln(cx-10,47,cx+10,47);ln(cx,43,cx,30,P.dim);rc(cx-2,32,5,3);}
   if(i===1){const middle=cx+Math.trunc(value*7/127);rc(cx-18,43,37,1,P.line);rc(cx-18,30,3,14);rc(middle,34,3,10);rc(cx+16,30,3,14);}
   if(i===2&&applied&&[applied.mode,applied.cutoff,applied.resonance].every(Number.isInteger)){const mode=applied.mode,cutoff=applied.cutoff,res=applied.resonance,left=cx-19,right=cx+19,pivot=left+7+Math.trunc(cutoff*24/127),pass=36,floor=46,peak=pass-Math.trunc(res*7/127);rc(left,floor,39,1,P.line);if(!mode){rc(left,38,39,1,P.dim);return;}if(mode===1){ln(left,pass,pivot-4,pass);ln(pivot-4,pass,pivot,peak);ln(pivot,peak,pivot+6,floor);ln(pivot+6,floor,right,floor);}else if(mode===3){ln(left,floor,pivot-6,floor);ln(pivot-6,floor,pivot,peak);ln(pivot,peak,pivot+4,pass);ln(pivot+4,pass,right,pass);}else{ln(left,floor,pivot-6,floor);ln(pivot-6,floor,pivot,peak);ln(pivot,peak,pivot+6,floor);ln(pivot+6,floor,right,floor);}}
   if(i===3){const depth=Math.trunc(value*14/127);ln(cx-18,31,cx-9,31);ln(cx-9,31,cx-6,31+depth);ln(cx-6,31+depth,cx+15,31);rc(cx+15,31,4,1);rc(cx-18,48,37,1,P.line);}
  }
  function power(q){r(211,4,24,11,P.gray);r(235,7,2,5,P.gray);r(212,5,22,9,P.bg);if(q===4){t('USB',223,5,8,P.ink,false,'center');}else if(Number.isInteger(q)&&q>=0&&q<=3){for(let i=0;i<q;i++)r(214+i*7,7,5,5,P.gray);}else t('?',223,4,10,P.gray,false,'center');}
  render=function(){priorRender();if(!S.shared)return;const c=S.shared,ids=['perform.tempo','perform.swing','perform.filter','perform.pump'];r(0,0,240,240,P.bg);slab(0,0,240,20);t('GLOBAL',5,1,14,P.amber,true);const sp=c.caller.sourcePart??c.caller.part,cp=c.musical.currentPatterns?.[sp],pending=c.musical.pendingPatterns?.[sp];t('P'+(Number.isInteger(cp)?cp+1:'?')+(Number.isInteger(pending)&&pending>=0?'>'+ (pending+1):''),97,3,11,P.gray,true,'right');t(c.transport.wait?'WAIT':c.scalars[ids[0]].storedRaw,179,c.transport.wait?3:0,c.transport.wait?11:16,c.transport.wait?P.amber:P.gray,false,'right');if(c.transport.run&&!c.transport.wait){ctx.fillStyle=P.value;ctx.beginPath();ctx.moveTo(184,4);ctx.lineTo(191,9);ctx.lineTo(184,14);ctx.fill();}if(c.transport.rec)r(195,6,5,5,'#f05e56');if(c.lifecycle.dirty)r(204,3,3,3,P.amber);power(c.power);
   t(S.g<0?'SETTINGS':GROUPS[S.g],8,25,12,P.ink,true);t(c.caller.partLabel||'HOST',232,26,10,P.gray,false,'right');
   if(S.modal){t(S.modal.question[0],120,55,15,P.value,true,'center');t(S.modal.question[1],120,80,11,P.gray,false,'center');t('PATCH: CANCEL',120,111,13,P.amber,true,'center');t('SCALE / PLAY: RESERVED',120,137,10,P.gray,false,'center');}
   else{const row=current();if(row?.text){t('PEER INTERFACE RESERVED',120,70,13,P.gray,true,'center');t('No shared peer backing',120,99,12,P.dim,false,'center');}else{const items=S.g<0?categories():rows(),pos=S.g<0?S.cat:S.pos,top=Math.max(0,pos-1);for(let k=0;k<2;k++){const item=items[top+k];if(!item)break;const y=49+k*43,chosen=top+k===pos;slab(3,y,234,40,chosen);t(item.name,10,y+5,12,chosen?P.ink:P.gray,chosen);const value=S.g<0?'>':item.act?'RESERVED':val(item);t(value,228,y+22,11,value==='UNAVAILABLE'?P.dim:P.amber,true,'right');}}
   t((c.transport.wait?'WAIT':c.transport.run?'RUN':'STOP')+' '+(c.transport.clockSource==='EXTERNAL'?'EXT':'INT'),8,142,10,P.gray);t(String(c.lifecycle.state||'UNKNOWN').toUpperCase(),232,142,10,P.amber,false,'right');}
   slab(3,158,234,11);t('PERFORM',8,158,11,P.amber);const vals=[c.scalars[ids[0]].storedRaw+'bpm',(50+Math.floor(c.scalars[ids[1]].storedRaw/4))+'%',c.quick?.filterText||'N/A',c.quick?.pumpText||'N/A'];['TEMPO','SWING','FILTER','PUMP'].forEach((name,i)=>{slab(i*60+3,171,54,55);t(name,i*60+30,172,11,P.gray,false,'center');glyph(i,c.scalars[ids[i]].appliedRaw,c.quick?.filterApplied);t(vals[i],i*60+30,211,15,P.amber,true,'center');});slab(0,229,240,11);t(S.modal?'PATCH CANCEL  GLOBE MASKED':S.g<0?'K2 SELECT  > ENTER  PATCH/GLOBE EXIT':'K2 SELECT  < / PATCH BACK  GLOBE EXIT',4,229,10,P.gray);
   const big=document.getElementById('large').getContext('2d');big.imageSmoothingEnabled=false;big.drawImage(document.getElementById('lcd'),0,0,720,720);
  };
  render();return api;
 };
})();
