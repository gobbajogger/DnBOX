"""Enumerate concrete reachable UI fixtures through the real host command interface.
No direct UI writes; page/list counts are queried from production page construction.
"""
import os,re,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
ENV=dict(os.environ,X0X_HOST_FIXED_CLOCK='1',X0X_HOST_LOAD='0',LC_ALL='C',TZ='UTC')

def run(commands,out):
    out=Path(out);out.mkdir(parents=True,exist_ok=True)
    script=out/'scene.x0x';script.write_text(commands+'\n')
    p=subprocess.run([str(ROOT/'build/host/x0x_host'),str(script),str(out)],env=ENV,cwd=ROOT,text=True,capture_output=True)
    if p.returncode: raise RuntimeError(p.stdout+p.stderr)
    return p.stdout

def state(commands,tmp):
    s=run(commands+'\nuistate',tmp)
    return {k:int(v) for k,v in re.findall(r'(\w+)=(-?\d+)',s)}

def catalog():
    """Finite equivalence classes, not all combinations of 128-valued parameters.
    Both themes, every constructed page, drum selection, menu row, help card,
    held-step and touch states. View-count changes are discovered, never omitted.
    """
    scenes=[]
    with tempfile.TemporaryDirectory() as d:
        def add(name,cmd,target=None):
            scenes.append(dict(id=name,commands=cmd+'\nwait 40',target=target))
        # ALGO's physical ordering is 909,808,BREAK,Chicago,Miami.
        for pi,part in enumerate(['909','808','break','chicago','miami']):
            start=f'wait 300\nturn ALGO {pi}\n'
            for view,key in [('seq','SEQ'),('para','EDIT'),('fx','FX'),('mix','LFO'),('home','HOME')]+([('gen','ARP')] if pi>=3 else []):
                cmd=start+f'tap {key}'
                n=state(cmd,d)['pages']
                for page in range(n):
                    c=cmd+f'\n'+f'tap {key}\n'*page
                    target=f'{part}-voice-00' if view=='para' and pi<2 else None
                    add(f'{part}-{view}-{page:02}',c,target)
                    add(f'{part}-{view}-{page:02}-touch',c+'\nspin K1 1')
                    if view in ('para','seq') and pi<2:
                        for voice in range(1,11):
                            add(f'{part}-{view}-{page:02}-voice-{voice:02}',c+f'\ntapkey b{voice}',f'{part}-voice-{voice:02}' if view=='para' else None)
                    if view=='seq': add(f'{part}-{view}-{page:02}-held',c+'\nkey w0 down')
                # Every deep-list row, including scrolling and actions (not invoked).
                c=cmd+'\ntap SEL'
                for row in range(state(c,d)['list_n']):
                    add(f'{part}-{view}-list-{row:02}',c+f'\nturn SELECT {row}')
        add('global-index','wait 300\ntap GLO')
        for section in range(state('wait 300\ntap GLO',d)['list_n']):
            c=f'wait 300\ntap GLO\nturn SELECT {section}\ntap SEL'
            for row in range(state(c,d)['list_n']): add(f'global-{section}-row-{row:02}',c+f'\nturn SELECT {row}')
        # KEYS + SCREEN > THEME: index 1 (AMBER) is the default; every other theme on each top-level view.
        for theme,name in enumerate(['green','amber','cyan','red','mono','light']):
            if theme==1: continue
            t=f'wait 300\ntap GLO\nturn SELECT 2\ntap SEL\nturn SELECT 2\nturn ALGO {theme-1}\ntap GLO\n'
            for view,key,pi in [('seq','SEQ',0),('para','EDIT',0),('para','EDIT',1),('para','EDIT',3),('para','EDIT',4),('gen','ARP',4),('seq','SEQ',2),('fx','FX',0),('mix','LFO',0),('home','HOME',0)]:
                add(f'theme-{name}-p{pi}-{view}',t+f'turn ALGO {pi}\ntap {key}')
            add(f'theme-{name}-global',t+'tap GLO')
        add('transport-playing','wait 300\ntapkey b0\ntapkey w0\ntapkey w4\ntap PLAY\nwait 777')
        add('transport-rec','wait 300\ntap REC\ntap PLAY\nwait 333')
        for btn in ['FX','SEL','ENV','LFO','EDIT','GLO','HOME','SAVE','ARP','SEQ','PLAY','REC','OCT-','OCT+']:
            add('help-'+btn.replace('+','up').replace('-','down'),f'wait 300\npress {btn}\nwait 1200')
        for p in range(2): add(f'song-{p}','wait 300\ntap HOME\ntap HOME\n'+'tap HOME\ntap HOME\n'*p)
        # Existing real interaction scenarios extend coverage to dialogs, cue, recording,
        # motion, transport and toasts. Each shot is an independent reproducible prefix.
        for scenario in ['tour','song','help','sound_matrix_chicago','sound_matrix_miami','sound_page_split','savefeedback','groove','patterns','clocksrc','perf','guard','jam','showcase','stereo']:
            lines=[]
            for line in (ROOT/f'tests/scenarios/{scenario}.x0x').read_text().splitlines():
                if line.startswith('shot '):
                    add('journey-'+scenario+'-'+Path(line.split()[1]).stem,'\n'.join(lines))
                elif not line.startswith(('wav ','wavstop','expect ')):
                    lines.append(line)
    return scenes
