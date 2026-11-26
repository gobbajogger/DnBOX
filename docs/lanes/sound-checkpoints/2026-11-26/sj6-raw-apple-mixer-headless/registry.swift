import AudioToolbox
let tests: [(String,OSType,OSType,OSType)] = [
  ("multi-channel mixer",kAudioUnitType_Mixer,kAudioUnitSubType_MultiChannelMixer,kAudioUnitManufacturer_Apple),
  ("stereo mixer",kAudioUnitType_Mixer,kAudioUnitSubType_StereoMixer,kAudioUnitManufacturer_Apple),
  ("matrix mixer",kAudioUnitType_Mixer,kAudioUnitSubType_MatrixMixer,kAudioUnitManufacturer_Apple),
  ("any mixer",kAudioUnitType_Mixer,0,0),
  ("any audio component",0,0,0)
]
for (label,type,subtype,vendor) in tests {
    var query = AudioComponentDescription(componentType:type,componentSubType:subtype,
        componentManufacturer:vendor,componentFlags:0,componentFlagsMask:0)
    print("\(label): \(AudioComponentFindNext(nil, &query) != nil)")
}
