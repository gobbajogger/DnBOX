from pathlib import Path
p = Path("firmware/src/app/engine.c")
s = p.read_text()
function = "static void apply_superj6_edits(void)\n"
assert s.count(function) == 1
s = s.replace(function, """/* Offline timing fixture: select a typed J60V2 preset on the loaded Juno owner. */
volatile uint32_t sound_chorus_probe_mode[NBASS];
static const sj6_factory_row sound_chorus_probe_row = {
    .present = ((1u << SJ6_ROW_AMOUNT) - 1u) | (1u << SJ6_ROW_CHORUS_I) | (1u << SJ6_ROW_CHORUS_II),
    .value = { .1f, .2f, .6f, .15f, .75f, 2.f/3.f, 1.f,
        [SJ6_ROW_CHORUS_I] = 0.f, [SJ6_ROW_CHORUS_II] = 1.f },
};
""" + function)
site = """        if (synth->pending_wave_mask || synth->pending_field_mask)
            apply_superj6_pending(part, owner);"""
assert s.count(site) == 1
s = s.replace(site, """        if (!sound_chorus_probe_mode[owner] && !synth->typed)
            (void)engine_superj6_factory_load(owner, &sound_chorus_probe_row);
""" + site + """
        if (synth->typed && !sound_chorus_probe_mode[owner])
            sound_chorus_probe_mode[owner] = part->dsp.juno.chorus.factory.mode;""")
p.write_text(s)
