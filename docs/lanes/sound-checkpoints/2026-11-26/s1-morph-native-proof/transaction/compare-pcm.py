"""Comparison of strict replay PCM; identity is an observed property, never inferred."""
from pathlib import Path
import hashlib,json,sys
a,b=(Path(x).read_bytes() for x in sys.argv[1:])
m=min(len(a),len(b))//2
diff=sum(a[i:i+2]!=b[i:i+2] for i in range(0,2*m,2))
first=next((i//2 for i in range(0,2*m,2) if a[i:i+2]!=b[i:i+2]),None)
print(json.dumps({'pcm_i16_values_left':len(a)//2,'pcm_i16_values_right':len(b)//2,
                  'mismatched_i16_in_common_length':diff,'first_mismatch_i16':first,
                  'hash_left':hashlib.sha256(a).hexdigest(),'hash_right':hashlib.sha256(b).hexdigest()}))
