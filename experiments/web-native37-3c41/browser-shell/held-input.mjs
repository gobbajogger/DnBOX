// Input ownership only. Native state and feedback are never inferred here.
export function createHeldInput(dispatch,command,getValue){let owner=null;return{get active(){return owner!==null},press(){if(owner!==null)return Promise.resolve();owner=getValue();return dispatch(command,owner,1)},release(){if(owner===null)return Promise.resolve();const original=owner;owner=null;return dispatch(command,original,0)}}}
export function presentationSizes(request,available){const bound=Math.max(0,Math.floor(available));const wanted=request==='fit'?Math.min(480,bound):Math.min(Number(request),bound);return{native:240*Math.max(1,Math.floor(wanted/240)),smooth:Math.max(1,wanted)}}
export function integerOriginPadding(position){return Math.ceil(position)-position}
