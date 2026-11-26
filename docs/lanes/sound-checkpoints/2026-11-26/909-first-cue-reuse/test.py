from read import decode
p=decode([0xc0000003|(1<<8)|(320<<11),0x3f800000,0x80000000,0xfffffffe])
assert p['half_ordinal']==320 and p['initial_free_half']==1 and p['final_free_half']==0
assert p['tap_909_pre_shape_l']==1 and p['tap_dma_final_int32_l']==-2
p=decode([0xc0000007|(1<<9)|(65535<<11),0,0,0])
assert p['selected_half_renderer_request']==[65280,65792] and p['half_ordinal']==65535 and p['complete_producer_marker_only']
for r in ([0,0,0,0],[73728,0,0,0],[0xa0000003,0,0,0],[0xb0000003,0,0,0],[0xffffffff,0,0,0],
          [0xc0000001,0,0,0],[0xc0000003|8,0,0,0],[0xc0000003|0x08000000,0,0,0],
          [0xc0000003,0x7f800000,0,0]):
    try:decode(r)
    except AssertionError:pass
    else:raise AssertionError(r)
print('PASS exact structure: 2 synthetic complete shape cases, 9 rejected records (including legacy/partial/overflow); no native producer or PCM claim')
