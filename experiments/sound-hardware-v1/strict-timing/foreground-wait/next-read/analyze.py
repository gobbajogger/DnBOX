"""Compare the observed physical clock, completed SPI waits and panel reads after matched DMA release numbers."""
import argparse
import json
from pathlib import Path

HZ=5760000000
TICKS_PER_US=HZ/1000000
TOKENS=(b'"event":"half"',b'"event":"foreground_context"',b'"event":"foreground_read"',
        b'"event":"foreground_spi_start"',b'"event":"dma"')


def load(path):
    halves={};events=[];irq={};schedules={}
    for line in path.open('rb'):
        if not any(key in line for key in TOKENS):continue
        r=json.loads(line);k=r['event'];seq=r['seq']
        if k=='half':halves[seq]=r
        elif k=='dma':schedules[seq]=r['scheduled_cycle']
        elif k=='foreground_context':irq[seq]=r
        else:events.append(r)
    assert len(halves)==648
    return halves,events,irq,schedules


def is_input(r):
    return r.get('name') in ('scene_read_input','scene_read_analog','adc')


def is_sensitive(r):
    return is_input(r) or r.get('name') in ('scene_read_clock','timer4')


def is_fader(r):
    return r.get('name')=='scene_read_analog' and 168 <= r['offset'] < 172


def readout(r):
    addr=r.get('offset',0)
    return dict(seq=r['seq'],cycle=r['cycle'],ms=round(r['cycle']*1000/HZ,6),
                name=r['name'],offset=addr,width=r['width'],value=r['value'])


def shot(series,anchor):
    halves,events,irq,schedule=series
    end=halves[anchor]['completion_cycle']
    horizon=459 if anchor==450 else 578
    after=[r for r in events if r['cycle']>=end and anchor<=r['seq']<=horizon]
    sensitive=next(i for i,r in enumerate(after) if is_sensitive(r))
    first=after[sensitive]
    panel=next(r for r in after if is_input(r))
    fader=next((r for r in after if is_fader(r)),None)
    before=after[:sensitive]
    transfers=[r for r in before if r['event']=='foreground_spi_start']
    status=[r for r in before if r.get('name')=='spi1_status']
    assert len(before)==len(status)+len(transfers)
    completed=[]
    for done in sorted({r['spi1_done_at'] for r in status}):
        seen=[r for r in status if r['spi1_done_at']==done]
        busy=next((r for r in seen if r['cycle']<done and not (r['value']&0x8000)),None)
        ready=next((r for r in seen if r['cycle']>=done and (r['value']&0x8000)),None)
        if busy and ready:
            completed.append(dict(deadline=done,first_busy_cycle=busy['cycle'],ready_cycle=ready['cycle']))
    return dict(release=anchor,guest_callback_us=halves[anchor]['last_us'],entry_cycle=irq[anchor]['cycle'],
                scheduled_cycle=schedule[anchor],completion_cycle=end,completion_ms=round(end*1000/HZ,6),
                first_sensitive_read=readout(first),first_panel_read=readout(panel),
                first_fader_read=readout(fader) if fader else None,
                before_first_read=dict(spi1_status_polls=len(status),transfers=[
                    dict(bytes=r['bytes'],start=r['cycle'],deadline=r['spi1_done_at']) for r in transfers],
                    completed_blocking_waits=completed))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('stock_trace',type=Path);p.add_argument('candidate_trace',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();stock=load(a.stock_trace);cand=load(a.candidate_trace)
    tests={}
    for n in (450,566,567):
        s=shot(stock,n);c=shot(cand,n);sw=s['before_first_read'];cw=c['before_first_read']
        sd=[r['deadline'] for r in sw['completed_blocking_waits']]
        cd=[r['deadline'] for r in cw['completed_blocking_waits']]
        nearby=min((abs(x-y) for x in sd for y in cd),default=None)
        dx=c['first_sensitive_read']['cycle']-s['first_sensitive_read']['cycle']
        tests[n]=dict(stock=s,candidate=c,first_read_delta_us=round(dx/TICKS_PER_US,6),
                      same_ordered_transfer_lengths=[r['bytes'] for r in sw['transfers']]==[r['bytes'] for r in cw['transfers']],
                      common_completed_spi_deadlines=sorted(set(sd)&set(cd)),
                      closest_completed_wait_deadline_us=None if nearby is None else round(nearby/TICKS_PER_US,6))
    a.output.write_text(json.dumps(dict(clock_hz=HZ,cases=tests),indent=2)+'\n')

if __name__=='__main__':
    main()
