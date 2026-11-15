/* SPDX-License-Identifier: GPL-3.0-only */
/* Exact existing Chicago 29x19 cache + native gfx.c cv_line raster; reused for
 * Juno's WAVE blend control. Raw shape cue only: no PWM/LFO/audio trace. */
function sourceWaveGlyph(q,x,y,ink){
 const ys=D.chicagoWave.wave_y_by_q[q],xx=Math.round(x),yy=Math.round(y);
 const rgb565=hex=>{const a=hex.match(/[a-f0-9]{2}/gi).map(v=>parseInt(v,16));return(a[0]>>3)<<11|(a[1]>>2)<<5|a[2]>>3;};
 const css=v=>'rgb('+((v>>11&31)*255/31|0)+','+((v>>5&63)*255/63|0)+','+((v&31)*255/31|0)+')';
 const before=g.getImageData(xx,yy,1,1).data,face=(before[0]>>3)<<11|(before[1]>>2)<<5|before[2]>>3,col=rgb565(ink);
 const mix=(a,b,t)=>((((a>>11)*(16-t)+(b>>11)*t)>>4)<<11)|(((((a>>5&63)*(16-t)+(b>>5&63)*t)>>4))<<5)|(((a&31)*(16-t)+(b&31)*t)>>4);
 rect(xx,yy+D.chicagoWave.zero,29,1,css(mix(face,col,3)));
 function pixelLine(x0,y0,x1,y1){let dx=Math.abs(x1-x0),sx=x0<x1?1:-1,dy=-Math.abs(y1-y0),sy=y0<y1?1:-1,err=dx+dy,guard=2000;while(guard--){rect(x0,y0,1,1,css(col));if(x0===x1&&y0===y1)break;const e2=2*err;if(e2>=dy){err+=dy;x0+=sx;}if(e2<=dx){err+=dx;y0+=sy;}}}
 for(let i=1;i<29;i++)pixelLine(xx+i-1,yy+ys[i-1],xx+i,yy+ys[i]);
}
