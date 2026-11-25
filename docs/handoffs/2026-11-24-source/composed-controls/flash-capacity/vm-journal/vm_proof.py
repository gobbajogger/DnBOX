#!/usr/bin/env python3
"""Offline candidate checkpoint geometry and power-cut model; no device or flash I/O."""
import json, struct, zlib
from dataclasses import dataclass
KiB=1024
SECTOR=4*KiB
PAGE=256
SOUND=7*12*32 + 2*12 + 4
SETTINGS=4 + 40 + 4
PANEL=4 + 14 + 7 + 7
assert (SOUND,SETTINGS,PANEL)==(2716,48,32)
PREFIX=16
DESC=16
BODYOFF=PAGE
MAGIC=b"DNVM"
ERASED=bytes([255])*SECTOR
# Sequence ties choose a single deterministic highest generation. This is a model, not a production wire format.
def crc(bs):return zlib.crc32(bs)
def split(v,n):
 return v+[b'']*(n-len(v))
def snapshot(sound,panel,settings,oldpanel=b'',oldsettings=b''):
 return (sound,panel,settings,oldpanel,oldsettings)
def rec(seq, entries):
 body=bytearray()
 for kind,piece in enumerate(entries,1):
  body+=struct.pack('<4I',kind,len(piece),crc(piece),0)
  body+=piece
 header=struct.pack('<4sII',MAGIC,seq,len(body))
 header+=struct.pack('<I',crc(header+body))
 assert len(header)==PREFIX
 assert len(body)<=SECTOR-BODYOFF
 return header,bytes(body)
def read(image):
 try:
  magic,seq,n,checksum=struct.unpack_from('<4sIII',image)
  if magic!=MAGIC or not (0<n<=SECTOR-BODYOFF):return None
  body=image[BODYOFF:BODYOFF+n]
  if crc(image[:12]+body)!=checksum:return None
  result=[];cursor=0
  while cursor<len(body):
   tag,m,check,reserved=struct.unpack_from('<4I',body,cursor)
   if (tag,reserved)!=(len(result)+1,0):return None
   cursor+=DESC
   if m>len(body)-cursor:return None
   payload=body[cursor:cursor+m]
   if crc(payload)!=check:return None
   result.append(payload);cursor+=m
  return seq,tuple(result)
 except (struct.error,ValueError):
  return None
def current(images):
 valid=[r for im in images if(r:=read(im))]
 return max(valid,key=lambda s:s[0]) if valid else None
def altered(size,seed):return bytes((i*73+seed*17)&255 for i in range(size))
def operation_images(obsolete, header, body):
 # Cut in the erase of whichever inactive sector the transaction will write.
 for n in range(SECTOR+1):
  yield ERASED[:n]+obsolete[n:],False
 # Cut after erase or in the sequence of NOR page programming calls.
 for n in range(len(body)+1):
  yield ERASED[:BODYOFF]+body[:n]+ERASED[BODYOFF+n:],False
 # Header last. A partial header may already be valid only if the unwritten
 # tail was equal to erased bits; in that case logical commit is already full.
 full=ERASED[:BODYOFF]+body+ERASED[BODYOFF+len(body):]
 for n in range(len(header)+1):
  yield header[:n]+full[n:],n==len(header)
def image(seq,entries):
 h,b=rec(seq,entries)
 return h+ERASED[len(h):BODYOFF]+b+ERASED[BODYOFF+len(b):]
def exercise(oldseq,obsolete,existing,incoming):
 active=image(oldseq,existing)
 outdated=image(oldseq-1,obsolete)
 target=rec(oldseq+1,incoming)
 expected={(oldseq,existing),(oldseq+1,incoming)}
 cuts=0
 for trial,finished in operation_images(outdated,*target):
  got=current((active,trial))
  assert got in expected,(cuts,got)
  if finished:assert got==(oldseq+1,incoming)
  cuts+=1
 return cuts

S=altered(SOUND,2); P=altered(PANEL,3); T=altered(SETTINGS,4)
O=snapshot(altered(SOUND,1),altered(PANEL,1),altered(SETTINGS,1))
C=snapshot(S,P,T,altered(PANEL,2),altered(SETTINGS,3))
edits={
  'sound update':snapshot(altered(SOUND,6),P,T,C[3],C[4]),
  'panel update':snapshot(S,altered(PANEL,6),T,P,C[4]),
  'settings update':snapshot(S,P,altered(SETTINGS,6),C[3],T),
}
modes=[
 ('Conservative: settings+panel A/B',2*SECTOR, (P,T,C[3],C[4])),
 ('All live values: sound/settings/panel A/B',4*SECTOR,C),
]
for desc,space,entries in modes:
 h,b=rec(1,entries)
 print(json.dumps(dict(mode=desc,body=len(b),span=BODYOFF+len(b),
                       tail_sector_bytes=SECTOR-BODYOFF-len(b),sector_app_or_asset_capacity=space)))
for label,case in edits.items():
 print(label,exercise(3,O,C,case),'power-cut positions preserve complete old or complete new snapshot')
for label in ('panel update','settings update'):
 print('conservative',label,exercise(3,O[1:]+(b'',b''),C[1:],edits[label][1:]),
       'power-cut positions preserve complete old or complete new snapshot')
