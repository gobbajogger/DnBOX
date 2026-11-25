#!/usr/bin/env python3
"""Inject only the isolated, CRC guarded XDATA font tail. Never package/flash."""
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
def change(old,new):
 global s
 assert s.count(old)==1,old
 s=s.replace(old,new)
change('    def link(header,companion,fittab):', '''    xdata_tables = {
        '.rodata.original_sound_chicago_params': 264,
        '.rodata.original_sound_miami_params': 216,
    }
    def link(header,companion,fittab,xcrc):''')
change("        s=script.replace('    XIP    (rx)'","        s=script.replace('    XIP    (rx)'") # Assert actual current wrapper.
a="        s=s.replace(' *(.rodata*)"
i=s.index(a)
s=s[:i]+'''        assert s.count('_xdata_end = .; } > XDATA') == 1
        tail=' . = ALIGN(4); cold_xdata_tail_start = .; '+''.join(' KEEP(*('+n+'))' for n in xdata_tables)+' cold_xdata_tail_end = .;'
        s=s.replace('_xdata_end = .; } > XDATA','_xdata_end = .;'+tail+' } > XDATA',1)
        # GNU ld assigns input sections in script order. The exact cold
        # selectors must run before .text's generic .rodata* collector.
        import re
        xm=re.search(r'^ +\.xdata.*?_xdata_end = \.;.*?} > XDATA[ \t]*$',s,re.M|re.S)
        assert xm and s.count(' .text :')==1
        xblock=xm.group(0)
        s=(s[:xm.start()]+s[xm.end():]).replace(' .text :',xblock+chr(10)+' .text :',1)
'''+s[i:]
change("+h3+' . = ALIGN(4); _etext'", "+h3+' cold_xdata_tail_crc = .; LONG(0x%08x); . = ALIGN(4); _etext'%xcrc")
change('    a=link(bytes(32),bytes(32),bytes(32))','''    a=link(bytes(32),bytes(32),bytes(32),0)
    mt=mp.read_text()
    def sym(n):
        return int(re.search(r'^\\s*(0x[0-9a-f]+)\\s+'+re.escape(n)+r' = \\.',mt,re.M)[1],16)
    lo,hi=(sym(n)-0x020A9000 for n in ('cold_xdata_tail_start','cold_xdata_tail_end'))
    assert sym('_xdata_end')-0x020A9000==7698
    assert lo==7700 and hi==8180 and len(a[5])==hi
    for n,size in xdata_tables.items():
        addr,count=(int(x,16) for x in re.search(r'^ '+re.escape(n)+r'\\s+(0x[0-9a-f]+)\\s+(0x[0-9a-f]+)',mt,re.M).groups())
        assert count==size and 0x020A9000+lo<=addr<=0x020A9000+hi-count,(n,addr,count)
    xcrc=zlib.crc32(a[5][lo:hi])''')
change('    z=link(h,h2,h3)','    z=link(h,h2,h3,xcrc)')
change('bytes(96)+z[0][n+96:]', 'bytes(100)+z[0][n+100:]')
change("    assert z[4]==h3+p3","    assert z[0][n+96:n+100]==struct.pack('<I',xcrc)\n    assert z[4]==h3+p3")
change('b.generate=lambda:None','b.CFLAGS+=["-DX0X_COLD_XDATA_TAIL_LAB=1"]\nb.generate=lambda:None')
Path(sys.argv[2]).write_text(s)
