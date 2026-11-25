"""Split the private corrected listening fixture into reproducible temporary halves."""
import hashlib
import io
from pathlib import Path
import sys
import wave
import zipfile

p=Path(sys.argv[1]); out=Path(sys.argv[2])
assert hashlib.sha256(p.read_bytes()).hexdigest()=="ff79d78320b6a04b11f0db3ec393ebe6ee91caa84f78e8afebda4ad73e0229f0"
out.mkdir()
with zipfile.ZipFile(p)as z:
    for i in ("001","062","113","128","278","435"):
        with wave.open(io.BytesIO(z.read(f"{i}-AB-original-then-compact.wav")))as w:
            assert w.getparams()[:4]==(2,2,44100,551250)
            body=w.readframes(264600)
            silence=w.readframes(22050)
            assert not any(silence)
            end=w.readframes(264600)
        for label,buf in [("original",body),("historical-compact",end)]:
            with wave.open(str(out/f"{i}-{label}.wav"),"wb")as o:
                o.setparams((2,2,44100,264600,"NONE","not compressed"))
                o.writeframes(buf)
