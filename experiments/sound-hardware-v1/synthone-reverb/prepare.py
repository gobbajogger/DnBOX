"""Build a reduced-storage alternative using only the repository's pinned RevSC source."""
from pathlib import Path
import re
import sys


def replace_once(text, before, after):
    assert text.count(before) == 1, before
    return text.replace(before, after)


def prepare(repo, out):
    fx = repo / 'tests/fixtures/synthone-factory/fullfx'
    rev = (fx / 'upstream/AudioKit/Core/Soundpipe/modules/revsc.c').read_text()
    rev = rev.replace('sp_revsc_', 'sp_compact_').replace('sp_compact_dl', 'sp_revsc_dl')
    rev = replace_once(rev, '#define DEFAULT_SRATE', '''/* Signed 8-bit mu-law with a symmetric +/-4 input range and exact zero. */
static int8_t encode(float value) {
    float mag = fminf(fabsf(value), 4.f);
    return (int8_t)copysignf(roundf(log1pf(255.f * mag / 4.f) * (127.f / logf(256.f))), value);
}
static float decode(const sp_revsc_dl* line, int position) {
    int code = ((const int8_t*)line->buf)[position];
    return copysignf(expm1f(fabsf((float)code) * (logf(256.f) / 127.f)) * (4.f / 255.f), (float)code);
}
#define DEFAULT_SRATE''')
    rev = replace_once(rev, '* (int) sizeof(SPFLOAT)', '* (int) sizeof(int8_t)')
    rev = replace_once(rev, 'sizeof(SPFLOAT) * lp->bufferSize', 'sizeof(int8_t) * lp->bufferSize')
    rev = replace_once(rev, 'lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)\n                                 - lp->filterState);', '((int8_t*)lp->buf)[lp->writePos] = encode((n & 1 ? ainR : ainL) - lp->filterState);')
    rev, count = re.subn(r'lp->buf\[(readPos(?: [+-] [12])?)\]', r'decode(lp, \1)', rev)
    assert count == 8
    (out / 'compact.c').write_text(rev)
    source = (fx / 'source_fx.hpp').read_text()
    source = replace_once(source, '        sp_revsc_compute(sp, reverbCostello,', '        measure_revsc(sp, reverbCostello,')
    (out / 'capture_fx.hpp').write_text(source)

if __name__ == '__main__':
    prepare(Path(sys.argv[1]), Path(sys.argv[2]))
