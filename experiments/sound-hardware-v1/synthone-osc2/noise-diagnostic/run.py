from pathlib import Path
import importlib.util, json, subprocess, sys
import numpy as np

root = Path.cwd()
factory, verified, out = [Path(p).resolve() for p in sys.argv[1:]]
out.mkdir(parents=True, exist_ok=True)
prior = root / 'experiments/sound-hardware-v1/synthone-osc2/filter-diagnostic'
spec = importlib.util.spec_from_file_location('replay', root / 'experiments/sound-hardware-v1/synthone-osc2/replay.py')
replay = importlib.util.module_from_spec(spec)
spec.loader.exec_module(replay)

def edit(filename, replacements):
    text = (prior / filename).read_text()
    for old, new in replacements:
        assert old in text, old
        text = text.replace(old, new, 1)
    (out / filename).write_text(text)

edit('original.cpp', [
    ('float values[] = {state.lfo1_1_0, state.lfo2_1_0};\n    CHECK(std::fwrite(values, sizeof(float), 2, out.lfo) == 2);',
     '''const auto route = state.parameters[noiseLFO];
    const float gain = route == 1.f ? state.lfo1_1_0 : route == 2.f ? state.lfo2_1_0 : route == 3.f ? state.lfo3_1_0 : 1.f;
    float values[] = {state.parameters[noiseVolume], gain, state.parameters[adsrPitchTracking], state.monoNote->noise->amp};
    CHECK(std::fwrite(values, sizeof(float), 4, out.lfo) == 4);
    uint32_t rand = state.sp->rand;
    CHECK(std::fwrite(&rand, sizeof rand, 1, out.lfo) == 1);'''),
])
edit('shipping.c', [
    ('static original_runtime_part part;', '''#include <math.h>
struct noise_frame { float level, lfo, tracking, original_amp; uint32_t original_seed; };
struct noise_frame s1_diag_noise[132300];
uint32_t s1_diag_seed[132300];
static original_runtime_part part;'''),
    ('assert(argc == 13);', 'assert(argc == 14);'),
    ('fread(s1_diag_lfo, sizeof s1_diag_lfo, 1, trace)', 'fread(s1_diag_noise, sizeof s1_diag_noise, 1, trace)'),
    ('assert(fclose(pcm) == 0);', '''assert(fclose(pcm) == 0);
    FILE *seed = fopen(argv[13], "wb");
    assert(seed && fwrite(s1_diag_seed, sizeof s1_diag_seed, 1, seed) == 1 && !fclose(seed));'''),
])
edit('s1_dry_part.c', [
    ('void s1_part_render(', '''static uint32_t owner_seed;
extern uint32_t s1_noise_state, s1_noise_multiplier, s1_noise_offset, s1_diag_seed[132300];
extern unsigned s1_diag_frame;
static uint32_t next_seed(uint32_t state)
{
    return (state * UINT32_C(1103515245) + UINT32_C(12345)) & UINT32_C(0x7fffffff);
}

void s1_part_render('''),
    ('memset(left, 0, frames * sizeof *left);\n    if (s1_diag_mode & 1u)', '''memset(left, 0, frames * sizeof *left);
    if (s1_diag_mode & 8u) {
        unsigned active = 0;
        unsigned limit = s1_diag_mode & 1u ? 1u : S1_CORE_VOICES;
        for (unsigned i = 0; i < limit; ++i) {
            s1_dry_begin_block(&part->voice[i]);
            active += part->voice[i].started;
        }
        s1_noise_multiplier = 1u;
        s1_noise_offset = 0u;
        for (unsigned i = 0; i < active; ++i) {
            s1_noise_multiplier *= UINT32_C(1103515245);
            s1_noise_offset = s1_noise_offset * UINT32_C(1103515245) + UINT32_C(12345);
        }
        s1_noise_state = next_seed(owner_seed);
        if (!active)
            for (unsigned i = 0; i < frames; ++i)
                s1_diag_seed[s1_diag_frame + i] = owner_seed;
    }
    if (s1_diag_mode & 1u)'''),
    ('} else for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {\n        s1_dry_begin_block(&part->voice[i]);\n        if (part->voice[i].started)', '''} else for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
        if (!(s1_diag_mode & 8u)) s1_dry_begin_block(&part->voice[i]);
        if (part->voice[i].started) {
            if (s1_diag_mode & 8u) s1_noise_state = owner_seed = next_seed(owner_seed);'''),
    ('    if (left != right) memcpy(right, left, frames * sizeof *left);', '''    if (s1_diag_mode & 8u)
        owner_seed = s1_diag_seed[s1_diag_frame + frames - 1];
    if (left != right) memcpy(right, left, frames * sizeof *left);'''),
    ('            );\n    }\n    if (s1_diag_mode & 8u)', '            );\n        }\n    }\n    if (s1_diag_mode & 8u)'),
])
edit('s1_dry_voice.c', [
    ('static __attribute__((noinline)) float oscillator_filter(', '''#include <math.h>
struct noise_frame { float level, lfo, tracking, original_amp; uint32_t original_seed; };
extern struct noise_frame s1_diag_noise[132300];
extern uint32_t s1_diag_seed[132300];
uint32_t s1_noise_state, s1_noise_multiplier, s1_noise_offset;
static unsigned local_sample;

static float diagnostic_noise(const s1_dry_voice *voice)
{
    const unsigned frame = s1_diag_frame + s1_diag_local + local_sample;
    const struct noise_frame *control = &s1_diag_noise[frame];
    uint32_t seed = s1_noise_state;
    if (s1_diag_mode & 16u) {
        seed = control->original_seed;
        if (seed == (frame ? s1_diag_noise[frame - 1].original_seed : 0u))
            return 0.f;
    }
    s1_diag_seed[frame] = seed;
    s1_noise_state = (seed * s1_noise_multiplier + s1_noise_offset) & UINT32_C(0x7fffffff);
    const float amplitude = s1_diag_mode & 32u ? control->original_amp : voice->velocity_squared;
    float sample = ((float)seed * (1.f / 2147483648.f) * 2.f - 1.f) * amplitude;
    sample *= control->level;
    sample *= control->lfo;
    const float pitch = log2f(voice->phase_increment * S1_DRY_SAMPLE_RATE);
    const float track = (pitch - 6.f) * 0.2f;
    const float k = 1.f - fminf(fmaxf(track, 0.f), 1.f);
    return sample * (control->tracking * k * k + (1.f - control->tracking));
}

static __attribute__((noinline)) float oscillator_filter('''),
    ('    excitation *= amp;', '    if (s1_diag_mode & 8u) excitation += diagnostic_noise(voice);\n    excitation *= amp;'),
    ('    mix[0] += dry_sample', '    local_sample = 0;\n    mix[0] += dry_sample'),
    ('        mix[i] += oscillator_filter', '        local_sample = i;\n        mix[i] += oscillator_filter'),
])

commands = json.loads((factory / 'factory-build-commands.json').read_text())
cc = next(cmd.copy() for cmd in commands if any(a.endswith('/preset_audio.cpp') for a in cmd))
for opt, name in [('-c', 'original.cpp'), ('-o', 'original.o'), ('-MF','original.d')]:
    cc[cc.index(opt)+1] = str(out/name)
subprocess.run(cc, check=True)
link = commands[-1].copy()
link[link.index(str(factory/'factory-6.o'))] = str(out/'original.o')
link[link.index('-o')+1] = str(out/'original')
subprocess.run(link, check=True)
flags = ['-std=c11','-O2','-fno-fast-math','-ffp-contract=off','-ffunction-sections','-fdata-sections',
         '-DX0X_HOST','-DX0X_SYNTHONE_PATCH=1','-DX0X_ORIGINAL_SYNTH=1','-DX0X_WOBBLE=0','-DX0X_DEV_WOBBLE=0',
         '-DORIGINAL_USE_SOFT_FMA','-DMIAMI_FX_NO_HOST_FENV','-DPROJECT_CRUSH_LFO=1','-DS1_PROJ_SUB=1',
         '-Iexperiments/original_synth','-Iexperiments/wobble','-Ifirmware/src','-Ifirmware/src/app',
         '-Ifirmware/src/dsp','-Ibuild/gen']
units = [out/Path(unit).name if Path(unit).name in ('s1_dry_part.c','s1_dry_voice.c') else unit
         for unit in [*replay.ORIGINAL_SYNTH_UNITS, *replay.ORIGINAL_SYNTH_BACKENDS]]
subprocess.run(['cc',*flags,out/'shipping.c',*units,'-Wl,--gc-sections','-lm','-o',out/'candidate'],check=True)

crossings = (19, 86, 130, 195, 213, 581)
full = {row['index']: row for row in map(json.loads,(verified/'rows.jsonl').read_text().splitlines())}
commands = factory/'commands/preset-commands.bin'
command_stream = np.fromfile(commands,dtype='<u4')[2:].reshape(694,289)
rows = []
for i in crossings:
    pcm, trace = out/f'original-{i}.f32', out/f'noise-{i}.bin'
    subprocess.run([out/'original', factory/'factory-inputs/public-morphs.f32',
                    factory/'factory-inputs/compact.bin', commands, str(i), pcm, trace], check=True)
    ref = np.fromfile(pcm,dtype='<f4').reshape(-1,2)
    assert replay.digest(ref.tobytes()) == full[i]['original_sha256']
    physical = dict(zip(command_stream[i,1::2],command_stream[i,2::2].view('<f4')))
    types = np.dtype([('level','<f4'),('route','<f4'),('track','<f4'),('amp','<f4'),('seed','<u4')])
    signal = np.fromfile(trace,dtype=types)
    assert len(signal) == 132300
    reference_seed = signal['seed']
    previous_seed = np.r_[np.uint32(0),reference_seed[:-1]]
    draws = np.flatnonzero(reference_seed != previous_seed)
    serial_step = ((previous_seed.astype(np.uint64) * 1103515245 + 12345) & 0x7fffffff).astype(np.uint32)
    assert np.array_equal(serial_step[draws], reference_seed[draws])
    assert np.count_nonzero(signal['track']) == 0
    for mode in (0, 1, 8, 9, 25, 57):
        pcm, editable, seed = (out/f'mode-{mode}-{i}.{suffix}' for suffix in ('f32','u8','seed'))
        subprocess.run([out/'candidate', verified/'panel-targets.f32', str(i), pcm, editable, str(mode),
                        str(int(physical[120])),str(physical[21]),'0','0','0','0',trace,seed], check=True)
        test = np.fromfile(pcm,dtype='<f4').reshape(-1,2)
        if mode == 0:
            assert replay.digest(test.tobytes()) == full[i]['stock_sha256']
        actual_seed = np.fromfile(seed,dtype='<u4')
        aligned = np.flatnonzero(actual_seed != reference_seed) if mode in (8, 9) else None
        row = dict(index=i, mode=mode, raw_peak=float(signal['level'].max()),
                   original_first_draw=int(draws[0]), original_last_draw=int(draws[-1]), original_draw_frames=len(draws),
                   scores=[replay.score(ref[:,c],test[:,c]) for c in (0,1)])
        if aligned is not None:
            row.update(seed_match_frames=len(reference_seed)-len(aligned), first_mismatch=int(aligned[0]) if len(aligned) else None)
        row['stock_sha256'] = replay.digest(test.tobytes())
        row['original_sha256'] = full[i]['original_sha256']
        rows.append(row)
        print(json.dumps(row),flush=True)
(out/'noise-study.json').write_text(json.dumps(rows, indent=2)+'\n')
