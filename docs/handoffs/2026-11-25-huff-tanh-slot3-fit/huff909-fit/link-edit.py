from pathlib import Path
p=Path('link-guarded-exact.py')
s=p.read_text()
s=s.replace("names=('crash','ride','hh')","names=('crash','ride','hh','huff_lut')")
s=s.replace("""    exact_fittab={""","""    extra_fdata.update({
"""+Path('ui-digests.receipt').read_text().rsplit('total exact bytes',1)[0]+"""    })
    streams={
        'crash':(19851,'c8b8b64a14351a07f4a4b4d2fdc340a5674e5d1d20be1b4c91fd8d3fd1b0ad22'),
        'ride':(21746,'6be8363921814de87fb5037f3604634ef68064f0f8a60226f7f0ffa56f5113b3'),
        'hh':(9286,'36877ea6c212f0741faafadb29e9da7133684723315770fb81fbc33f373c9c3d'),
        'huff_lut':(64,'3f72e3302a803c88212bf4babde1d93833f39c39695aee3e33bc97b2af744a19'),
    }
    cold909_end=32+sum(n for n,_ in streams.values())
    exact_fittab={""")
s=s.replace('55307+sum(relocated.values())','cold909_end+sum(relocated.values())').replace('0x020E1000+55307','0x020E1000+cold909_end')
s=s.replace("        off=addr-0x020E1000\n        assert off>=32 and len(data)==size and z[-2][off:off+size]==data","""        off=addr-0x020E1000
        assert 32<=off<=cold909_end-size and (size,hashlib.sha256(data).hexdigest())==streams[name]
        assert len(data)==size and z[-2][off:off+size]==data""")
s=s.replace("'-DX0X_COLD_FITTAB_LAB=1']","'-DX0X_COLD_FITTAB_LAB=1','-DX0X_909_HUFF_DIAG=1']")
p.write_text(s)
