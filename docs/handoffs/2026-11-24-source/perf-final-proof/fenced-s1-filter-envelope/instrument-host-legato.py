from pathlib import Path
p = Path('firmware/src/app/engine.c')
s = p.read_text()
p.write_text(s+'\nconst s1c *host_s1_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }\n')
p = Path('firmware/src/dsp/s1c/s1c.c')
s = p.read_text()
f = 'if (s->t[LEGATO] == 0.0f) voice_release(s, v, 1);'
assert s.count(f) == 2
p.write_text(s.replace(f, '''if (s->t[LEGATO] == 0.0f) {
            extern void host_s1_retrigger(const s1c *);
            host_s1_retrigger(s); voice_release(s, v, 1);
        }'''))
