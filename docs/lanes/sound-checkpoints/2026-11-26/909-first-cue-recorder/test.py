from read import decode
base=[3,65536,256|0x600,0,0,128,0x80,10800,1,2,3,4]
assert decode(base)['selected_half_renderer_request']==[65536,66048]
for k,v in [(0,1),(1,65537),(2,256),(5,65536),(6,0),(6,0x480),(7,0)]:
 row=base.copy();row[k]=v
 try:decode(row)
 except AssertionError:pass
 else:raise AssertionError((k,v))
print('PASS one structural complete record and seven independent fail-closed negatives; synthetic only')
