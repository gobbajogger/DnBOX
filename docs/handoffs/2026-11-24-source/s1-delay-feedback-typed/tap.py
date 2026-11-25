from pathlib import Path
p=Path('firmware/src/dsp/s1c/s1c.c');s=p.read_text()
a='        if (dd > 1) {\n        /* lines run'
assert s.count(a)==1
s=s.replace(a,'''        {
            extern void host_feedback_tap(const s1c*,float,float,int);
            host_feedback_tap(s,fbk,T,dd);
        }
'''+a)
p.write_text(s)
