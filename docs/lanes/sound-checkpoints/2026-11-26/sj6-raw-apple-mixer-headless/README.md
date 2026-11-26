# Independent Apple mixer format preflight

The observed SuperJ6 original constructs an AVAudioMixerNode. The following
isolated tests target only the standard Apple framework, no original app,
personal audio, device play, security changes, gain fitting, or private
AU data. Their intended scope is unity/format/fan-in behavior with 1 or
4 input buses and authentic external API only. A completed synthetic
test could falsify a universal built-in mixing rule. Even a success
would not observe the original SuperJ6 render topology or private
oscSection output.

Run registry.swift first with the standard compiler. In the available
headless execution, all five AudioComponent lookups returned nil,
including wildcard. The corresponding mixer.swift compiled, but a
standalone AVAudioMixerNode init raised AVFAudio “required condition
is false: comp != nullptr” before any sample render. Retain this boundary:
there are no PCM rows and no mixing law, causal gain, or private-node
claim. Do not retry this on the same failing registry or make a new
Mac dependency for the existing signed render workflow.
