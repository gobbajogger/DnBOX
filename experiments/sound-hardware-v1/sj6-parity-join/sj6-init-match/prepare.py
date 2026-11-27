from pathlib import Path
d=Path(__file__).resolve().parent
s=d.parent/'sj6-parity-suite'
a=(s/'render.c').read_text()
a=a[:a.index(' uint8_t room[SJ6_ROOM_COUNT]')]+a[a.index(' unsigned f=0,e=0;'):]
a=a.replace(' REQUIRED(argc==2 && (!strcmp(argv[1],"0") || !strcmp(argv[1],"1")));\n const unsigned wet=argv[1][0]==\'1\';',' REQUIRED(argc==1);\n (void)argv;')
a=a[:a.index(' fprintf(stderr,"pass runtime')]+''' fprintf(stderr,"pass unmodified compact Init, frames=%u events=%u\\n",f,e);
 return 0;
}
'''
(d/'render.c').write_text(a)
