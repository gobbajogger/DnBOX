"""Exact current original leaf invocation; no source/audio exports."""
from pathlib import Path
import numpy as np
import j60, macho
from emu import Emu
NEW='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
def instantiate(on):
    modern=Path('sj6-modern-owner/probe.py')
    e={}
    exec(compile(modern.read_text().split('def trial(')[0],str(modern),'exec'),e)
    assert e['pin'](e['BIN'])==NEW and macho.EXPECTED_SHA==NEW
    if Emu._image is not None: assert Emu._image.sha==NEW
    device=Emu(str(e['BIN']))
    for attr,fn in {'CTOR':'C1Ev','INIT':'4initEid','SETP':'12setParameterEyfb','GETP':'12getParameterEy','PROC':'7processEjj'}.items():
        setattr(j60,attr,device.im.sym_by_name['__ZN14J60V2ChorusDSP'+fn])
    leaf=j60.J60(device,1,1.,1.,1.,sr=44100)
    assert [leaf.get(i) for i in range(5)]==[1.,1.,1.,1.,0.]
    if not on: device.wr(leaf.obj+0x29,b'\0')
    assert device.rd(leaf.obj+0x29,1)==bytes([int(on)])
    assert not device.unknown,device.unknown
    return leaf
