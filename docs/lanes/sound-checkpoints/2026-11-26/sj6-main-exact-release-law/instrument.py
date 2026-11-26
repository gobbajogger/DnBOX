import sys
from pathlib import Path
c,d=map(Path,sys.argv[1:]);s=c.read_text();k='static uint32_t lab_sub_pcm'; assert s.count(k)==1
s=s.replace(k,'''static struct {unsigned frame,key,slot,gate,stage;float amp,sub,mix;} rows[131072];
static unsigned trace_n;
__attribute__((visibility("default"))) const void *release_trace(void){return rows;}
__attribute__((visibility("default"))) unsigned release_trace_count(void){return trace_n;}
extern unsigned tracker_frame(void);
'''+k)
k='                    mix[l]+=velocity*amp*y;\n                    ++ota_count;';assert s.count(k)>=1
s=s.replace(k,'''                    mix[l]+=velocity*amp*y;
                    assert(trace_n<131072);
                    rows[trace_n++]=(__typeof__(rows[0])){tracker_frame(),v->note,i,v->gate,lv->amp.stage,amp,sub_y,y};
                    ++ota_count;''',1)
c.write_text(s)
s=d.read_text();k='void center_meter(int enabled)';assert s.count(k)==1
d.write_text(s.replace(k,'unsigned tracker_frame(void){return accept+refuse;}\n'+k))
