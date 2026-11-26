"""Read-only dry run. No emulator execution, transport or fake event join."""
import hashlib,json,re,sys
from pathlib import Path
W=Path(__file__).resolve().parents[1]
pin='fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017'
sha=lambda b:hashlib.sha256(b).hexdigest()
exe=(W/'restored-gate/fm1-live-ba83317a').read_bytes()
assert sha(exe)==pin
schema={
 'dma':['seq','scheduled_cycle','observed_cycle','deadline_cycle','address','words'],
 'irq_enter':['seq','cycle','depth','snapshot'],
 'store':['seq','name','offset','width','value'],
 'half':['seq','entry_cycle','completion_cycle','completion_uncertainty_cycles','dma_seq_at_completion','last_us_writes','last_us','peak_delta','invalid'],
}
for event,fields in schema.items():
 at=exe.index(f'"event":"{event}"'.encode())
 section=exe[at:at+512].split(b'{"event":',1)[0]
 assert all(f'"{f}"'.encode() in section for f in fields),(event,fields)
for needle in (b'FM1_AUDIO_TRACE',b'FM1_DNBOX_MAP',b'FM1_DNBOX_TRACE',b'global_slice_clock',b'expected_profile_not_artifact_attestation'):
 assert needle in exe
assert b'FM1_NO_SKIP' in exe
paths={m:W/f'widen-native-verification/observed-{m}-refused.raw.log' for m in ('no-skip','protected')}
refusal={m:p.read_text().strip() for m,p in paths.items()}
assert 'no timing/fault overrides' in refusal['no-skip']
assert 'changes protected package bytes below0x9D000' in refusal['protected']
scene=(W/'strict-warm-j60/verify/summary.json').read_bytes()
assert sha(scene)=='d35df183d41eb0f38db6e54a55b832806a840c1098d66fc0dbce63ecf60e93eb'
s=json.loads(scene)
rows=[dict(model=o['model'],preflight_owner_count=o['prior_to_tape_owner'],first_healthy_reply_ms=o['first_valid_owner_ms'],frames=o['raw_result']['frames']) for o in s['result']]
assert rows==[
 dict(model='elite',preflight_owner_count=0,first_healthy_reply_ms=2998,frames=319488),
 dict(model='pro',preflight_owner_count=0,first_healthy_reply_ms=3008,frames=319488)]
print(json.dumps(dict(eligible=False,runner=pin,existing_observer_embedded_fields=schema,exact_old_refusals=refusal,current_warm_capture=rows,
 observation_only=['DMA transfer cycle/address/word count in supported observer mode','ALNK callback completion/snapshot in supported observer mode','USB bus time and bytes','final RAM','DAC PCM relative index'],
 no_anchor=['protected warm candidate is not admitted to passive observer','USB IN/OUT is not typed application acknowledgement by producer frame','observer schema lacks scripted event publication/application identity and PCM read frame link'],
 undocumented_audio_trace='binary contains FM1_AUDIO_TRACE; no verified current emitted schema for applied events or DAC capture-frame ownership'),indent=2))
sys.exit(2)
