"""Big-integer literal reference against portable limb C++; supplied test tapes only."""
import sys,struct
from entropy_reference import Tape,table12
lines=open(sys.argv[1]).read().splitlines();assert len(lines)==1000
for i,line in enumerate(lines):
 def words():
  yield 0
  word=i+1
  while True:
   word=(word*6364136223846793005+1442695040888963407)&((1<<64)-1)
   yield word
 tape=Tape(words());table=table12(tape)
 expected=f'{i} {tape.requests}'+''.join(' '+struct.pack('>f',f).hex()for f in table)
 assert line==expected,(i,line[:80],expected[:80])
print('1000 entropy tapes,256000 output words and exact request counts match independent BigInt equations; no original RNG identity')
