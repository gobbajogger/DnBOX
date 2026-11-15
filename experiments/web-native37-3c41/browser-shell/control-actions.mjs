// Pair momentary panel edges even when a down request may have been accepted but its reply was lost.
export async function tapWithRelease(send,button){
 if(typeof send!=='function'||!Number.isInteger(button)||button<0||button>13)throw new TypeError('invalid native button tap');
 let downError=null,result;
 try{await send('button',button,1);}catch(error){downError=error;}
 try{result=await send('button',button,0);}catch(error){if(downError)throw new AggregateError([downError,error],'Button press and release transport failed.');throw error;}
 if(downError)throw downError;
 return result;
}
