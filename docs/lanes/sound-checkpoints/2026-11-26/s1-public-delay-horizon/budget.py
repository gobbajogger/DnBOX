import json,math
sr=44100
for t in (.21621620655059814,.5615994930267334,2.5):
 sec=5*t; full4=2*(math.ceil(2*t*sr)+math.ceil(t*sr));full3=2*math.ceil(2*t*sr)+math.ceil(t*sr)
 for room in (4252,9068//2,22000):
  # Optimistic three effective histories before all state, FX or codec metadata.
  h=3*math.ceil(.032*sr)*4
  low_bps=max(0,math.floor((room*8-h)/(4*sec)))
  lo=math.ceil((sr/8)*sec)*4
  print(json.dumps(dict(seconds=t,capacity_bytes=room,full_original_f32_bytes=4*full4,effective_fold_f32_bytes=4*full3,early_high_32ms_payload_only_bytes=h/8,low_pool_samples_per_sec_with_high_reserve=low_bps,low_max_Nyquist_before_filter_hz=low_bps/2,lower_band_8x_compact_total_payload_bytes=(lo+h)/8,full_rate_4bit_fold_payload_bytes=full3/2)))
