import json,cmath,math,sys
x=json.load(open(sys.argv[1]))
a,b,c=float.fromhex('0x1.ffa2ap-1'),float.fromhex('0x1.10427p-19'),float.fromhex('0x1.ff4564p-1')
def hp(h):
    z=cmath.exp(-2j*math.pi*h/44100)
    return a*(1-z)**2/((1-z)*(1-c*z)+b*z)
def dc(h,p):
    z=cmath.exp(-2j*math.pi*h/44100)
    return (1-z)/(1-p*z)
def wrap(d):return (d+180.)%360.-180.
def gd(z):return cmath.phase(z)*180/math.pi
out={}
for key,profile in [('root36',x['original']),('root54',x['established_next_zone']['original'])]:
    h=profile.get('steady_f_Hz',profile.get('Hz'))
    model=lambda f:hp(f)*dc(f,float.fromhex('0x1.fd70a4p-1'))*dc(f,.999)
    m=model(h)
    p=profile.get('harmonics_relative_phase_deg')
    levels=profile.get('harmonics_relative_magnitude_dBc')
    if p is None:
        p=profile['H3_H5_H7_H9_relative_degrees']
        levels=profile['H3_H5_H7_H9_dBc']
    else:
        p=[p[j-1] for j in [3,5,7,9]];levels=[levels[j-1] for j in [3,5,7,9]]
    rows=[]
    for k,term,mag in zip([3,5,7,9],p,levels):
        d=wrap(gd(model(k*h))-k*gd(m))
        ratio=20*math.log10(abs(model(k*h)/m))
        rows.append(dict(harmonic=k,terminal_phase_deg=term,source_equivalent_relative_phase_deg=wrap(term-d),
                         suffix_relative_phase_deg=d,terminal_dBc=mag,
                         source_equivalent_dBc=mag-ratio,suffix_relative_gain_dB=ratio))
    out[key]=dict(frequency_hz=h,h1_suffix_phase_deg=gd(m),h1_gain_dB=20*math.log10(abs(m)),rows=rows)
print(json.dumps(out,indent=2))
