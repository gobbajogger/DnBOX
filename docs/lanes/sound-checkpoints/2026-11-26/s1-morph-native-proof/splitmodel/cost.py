from pathlib import Path
import sys
p=Path(sys.argv[1]);t=p.read_text()
a='static uint32_t eng_job_cost[JB_N];'
i=t.index(a)
t=t[:i]+'''static uint32_t eng_808_prefix_cost, eng_808_full_cost; /* measured ticks per 256 */
'''+t[i:]
a='''    uint32_t c, e = eng_job_cost[j];
    if (n < 64u)
        return;                                         /* a short stretch is mostly set-up */
    c = ticks * 256u / n;
    eng_job_cost[j] = c > e ? c : e - ((e - c) >> 4);'''
b='''    uint32_t *slot = j==JB_808 && !eng_808_split ? &eng_808_full_cost : &eng_job_cost[j];
    uint32_t c, e = *slot;
    if (n < 64u)
        return;                                         /* a short stretch is mostly set-up */
    c = ticks * 256u / n;
    *slot = c > e ? c : e - ((e - c) >> 4);'''
assert t.count(a)==1;t=t.replace(a,b)
a='''    if (eng_808_split && !eng_808_body)
        d808_body_run();'''
b='''    if (eng_808_split && !eng_808_body) {
        uint32_t begin=plat_ticks24(), n=eng_job_arg&255u;
        d808_body_run();
        if (!n) n=256;
        if (n>=64) {
            uint32_t c=(plat_ticks24()-begin)*256u/n, e=eng_808_prefix_cost;
            eng_808_prefix_cost=c>e?c:e-((e-c)>>4);
        }
    }'''
assert t.count(a)==1;t=t.replace(a,b)
a='static uint32_t plan_cost(uint32_t m, const uint32_t *c)'
assert t.count(a)==1;t=t.replace(a,'static uint32_t plan_cost(uint32_t m, const uint32_t *c, uint32_t body, uint32_t full)')
a='''    int k, j;
    k = (int)(m & 1u);'''
b='''    int k, j;
    uint32_t prefix=(m && !(m&(1u<<JB_808))) ? body : 0;
    t[1]+=prefix;
    k = (int)(m & 1u);'''
assert t.count(a)==1;t=t.replace(a,b)
a='''        x = c[j] >> 4;
        t[k] += c[j] - x;'''
b='''        uint32_t cost=j==JB_808 && !prefix ? full : c[j];
        x = cost >> 4;
        t[k] += cost - x;'''
assert t.count(a)==1;t=t.replace(a,b)
a='''    best &= allowed;
    bc = plan_cost(best, c);'''
b='''    best &= allowed;
    uint32_t body=0,full=0;
    if (act&(1u<<JB_808)) {
        body=eng_808_prefix_cost*nn/256u;
        full=eng_808_full_cost*nn/256u;
        if(!full) full=c[JB_808]+body;
        if(!c[JB_808]) c[JB_808]=full>body?full-body:0;
    }
    bc = plan_cost(best, c,body,full);'''
assert t.count(a)==1;t=t.replace(a,b).replace('plan_cost(m, c);','plan_cost(m, c,body,full);')
p.write_text(t)
