#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Authenticate canonical owner text without pretending pages conform."""
import argparse,hashlib,json,os,stat
from pathlib import Path
CANONICAL_PATH='docs/ui/UI-SOURCE-OF-TRUTH.md'
CANONICAL_SHA256='868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6'
CANONICAL_BYTES=2891
MAX_DESCRIPTOR_BYTES=16384

def read_regular(path,limit):
 """Bounded, nonblocking open and fstat before any read; reject pipes/devices."""
 flags=os.O_RDONLY|os.O_NONBLOCK|getattr(os,'O_CLOEXEC',0)|getattr(os,'O_NOFOLLOW',0)
 try:fd=os.open(path,flags)
 except OSError as e:raise ValueError('bounded regular file unavailable') from e
 try:
  info=os.fstat(fd)
  if not stat.S_ISREG(info.st_mode) or info.st_size>limit:
   raise ValueError('input must be a bounded regular file')
  with os.fdopen(fd,'rb',closefd=False) as stream:data=stream.read(limit+1)
  if len(data)>limit:raise ValueError('regular input exceeds size limit')
  return data
 finally:os.close(fd)

def read_descriptor(path):
 try:return json.loads(read_regular(path,MAX_DESCRIPTOR_BYTES))
 except (UnicodeError,json.JSONDecodeError) as e:raise ValueError('invalid canonical descriptor JSON') from e

def authenticate(root,descriptor):
 root=Path(root).resolve()
 if descriptor.get('path')!=CANONICAL_PATH or descriptor.get('sha256')!=CANONICAL_SHA256 or descriptor.get('bytes')!=CANONICAL_BYTES:
  raise ValueError('descriptor must name the trusted exact canonical owner path/pin')
 path=(root/CANONICAL_PATH).resolve()
 if not path.is_relative_to(root):raise ValueError('canonical path escapes repository')
 data=read_regular(path,CANONICAL_BYTES)
 actual=hashlib.sha256(data).hexdigest()
 if len(data)!=CANONICAL_BYTES or actual!=CANONICAL_SHA256:
  raise ValueError('canonical owner document bytes/hash differ')
 return actual

def report_binding(report,expected):
 if report.get('canonicalSpecSHA256')!=expected:
  return ['report lacks exact canonical owner-spec binding']
 return []

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--root',type=Path,required=True);p.add_argument('--descriptor',type=Path,required=True)
 a=p.parse_args();digest=authenticate(a.root,read_descriptor(a.descriptor))
 print('CANONICAL_TEXT_IDENTITY_VERIFIED',digest)
 print('PAGE_CONFORMANCE_NOT_RUN: document identity is not visual/runtime acceptance')
 return 0
if __name__=='__main__':raise SystemExit(main())
