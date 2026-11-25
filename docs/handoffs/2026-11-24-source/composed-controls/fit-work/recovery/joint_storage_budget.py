"""Sector arithmetic for physical alternatives; offline, no flash I/O."""
from dataclasses import dataclass
import json

K=1024
S=4*K
def align(n):
    return ((n+S-1)//S)*S

SLOT1=80*K
SLOT2=120*K
SLOT3=32*K
DIRECTORIES=2*S
BANKS=180224  # exact removable banks, aligned; does not include directories
BRK1=55239   # measured packed bytes incl slot headers, 256 B restart
BRK2=46037
D9=57120    # conditioned total FDATA, same 909 ADPCM sample bytes
CK=27780    # FAST32 diagnostic Slot3 payload, not a separate stock allocation

@dataclass(frozen=True)
class Layout:
    description: str
    available: int
    immovable: tuple
    break_extents: tuple
    optimistic: bool=False
    def capacity(self,reserve,copacked=False):
        sizes=self.immovable+self.break_extents
        assert reserve>=0 and reserve%S==0
        payload=align(sum(sizes)) if copacked else sum(align(n)for n in sizes)
        rest=self.available-payload-DIRECTORIES-reserve
        assert rest%S==0
        return max(0,rest)

def scenarios():
    return [
        Layout("Current Slot2, unchanged lossless defaults",SLOT2,(61434,),(50061,)),
        Layout("Slot2 with already measured lossless audio savings",SLOT2,(D9,),(BRK2,)),
        Layout("All three slots pooled, all current sound assets preserved",SLOT1+SLOT2+SLOT3,(D9,CK),(BRK1,BRK2),True),
        Layout("All three pooled; assume all non-BREAK companions moved entirely elsewhere",SLOT1+SLOT2+SLOT3,(),(BRK1,BRK2),True),
        Layout("Slot2 alone; all non-BREAK assets moved, assume no BREAK in Slot2",SLOT2,(),(),True),
    ]
def report():
    rows=scenarios()
    ans=[]
    for r in rows:
        cap=r.capacity(0)
        tight = r.capacity(0, copacked=True) if r.optimistic else cap
        ans.append(dict(name=r.description,hypothetical=r.optimistic,preset_capacity_whole_extents_bytes=cap,
                    preset_capacity_ideal_copacked_before_reserve_bytes=tight,
                    deficit_ideal_copacked_before_reserve_bytes=BANKS-tight,
                    deficit_before_update_reserve_bytes=BANKS-cap,
                    required_compression_before_update_reserve_pct=round(100*(1-cap/BANKS),1)))
    assert [r.capacity(0)//K for r in rows]==[0,8,36,120,112]
    assert BANKS-rows[3].capacity(0)==56*K
    assert rows[3].capacity(0,True)==124*K and BANKS-rows[3].capacity(0,True)==52*K
    print(json.dumps(ans,indent=2))

if __name__=="__main__":report()
