from pathlib import Path
p=Path('firmware/src/app/ui.c');s=p.read_text()
def one(a,b):
 global s
 assert s.count(a)==1,(a,s.count(a))
 s=s.replace(a,b)
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''/* Test-only teaching knob. No persistent Sound or stock admission. */
enum {R_ENV_TEACH=127};
extern int fixture_s1_env(unsigned owner,int position);
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
i=s.index('case R_S1LINK: {')
s=s[:i]+'case R_ENV_TEACH: {static const char *const names[]={"OFF","LFO 1","LFO 2","BOTH"};static const x0x_param_t p={"ENV LFO",3,0,names};return &p;}\n    '+s[i:]
i=s.index('case R_S1LINK: {',s.index('int pref_get(pref_t'))
s=s[:i]+'case R_ENV_TEACH: return fixture_s1_env(r.b,-1);\n    '+s[i:]
one('        add_page_refs(emit, pages[row].title, refs, 0, 0);','''        if (kind==R_S1PATCH && pages[row].title==PT_FILTER_MODE && live_s1_mod(&proj.sound,owner))
            refs[3]=PR(R_ENV_TEACH,T_303,owner,0);
        add_page_refs(emit, pages[row].title, refs, 0, 0);''')
one('static void turn_ref(pref_t r, int e, int knob)\n{','''static void turn_ref(pref_t r, int e, int knob)
{
    if(r.kind==R_ENV_TEACH) {
        int pos=fixture_s1_env(r.b,-1)+(e>0)-(e<0);
        if(pos>=0 && pos<=3)(void)fixture_s1_env(r.b,pos);
        return;
    }''')
p.write_text(s)
