"""Find audio callbacks that could finish earlier inside the same foreground SPI wait."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf',type=Path)
    parser.add_argument('trace',type=Path)
    parser.add_argument('result',type=Path)
    parser.add_argument('--limit',type=int,default=10909)
    args=parser.parse_args()
    symbols=[]
    for line in subprocess.check_output(['nm','-S',str(args.elf)],stderr=subprocess.DEVNULL,text=True).splitlines():
        fields=line.split()
        if len(fields)==4 and fields[2] in ('t','T'):
            begin,size=int(fields[0],16),int(fields[1],16)
            symbols.append((begin,begin+size,fields[3]))
    symbols.sort(reverse=True)
    context={};entries={};halves={};hz=None;finish=None
    for line in args.trace.open('rb'):
        event=json.loads(line);kind=event['event']
        if kind=='finish':finish=event
        if kind=='observer':hz=event['hz']
        if kind=='foreground_context': context[event['seq']]=event
        if kind=='irq_enter':entries[event['seq']]=event
        if kind=='half':halves[event['seq']]=event
    assert hz==5760000000 and finish and len(halves)==finish['completed'] and finish['resets']==0
    pending=set() if finish['pending_seq'] is None else {finish['pending_seq']}
    assert set(context)==set(entries)==set(halves)|pending, 'missing IRQ context or unexplained completion'
    rows=[]
    for seq,half in sorted(halves.items()):
        c=context[seq];e=entries[seq];pc=c['interrupted_pc']
        assert c['cycle']==e['cycle']==half['entry_cycle']
        function=next((name for lo,hi,name in symbols if lo<=pc<hi),'unresolved')
        remaining=max(c['spi1_done_at']-c['cycle'],0)
        protected=(e['depth']==1 and function=='fm1_lcd_wait' and c['spi1_done_at']>=half['completion_cycle'])
        rows.append(dict(release=seq,entry_ms=round(c['cycle']*1000/hz,6),last_us=half['last_us'],
                         depth=e['depth'],interrupted_pc=hex(pc),function=function,
                         spi_remaining_us=round(remaining*1000000/hz,6),wire_outlasts_irq=c['spi1_done_at']>=half['completion_cycle'],
                         interrupted_lcd_wait=e['depth']==1 and function=='fm1_lcd_wait',
                         resume_still_waiting=protected))
    peak=[r for r in rows if r['last_us']>args.limit]
    measured=dict(source_elf_sha256=sha(args.elf),trace_sha256=sha(args.trace),clock_hz=hz,target_us=args.limit,
                  in_flight_at_shot_end=finish['pending_seq'], completed_halves=len(rows),over_target_halves=len(peak),worst_us=max(r['last_us'] for r in rows),
                  all_entered_in_lcd_wait=sum(r['interrupted_lcd_wait'] for r in rows),
                  all_resumed_inside_same_wait=sum(r['resume_still_waiting'] for r in rows),
                  peak_entered_in_lcd_wait=sum(r['interrupted_lcd_wait'] for r in peak),
                  peak_resumed_inside_same_wait=sum(r['resume_still_waiting'] for r in peak),
                  over_target=peak)
    args.result.write_text(json.dumps(measured,indent=2)+'\n')
    print(json.dumps({k:v for k,v in measured.items() if k!='over_target'}))

if __name__=='__main__':
    main()
