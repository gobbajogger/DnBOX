# Extract note-generation verbatim except that the clock owns resets and edges.
from pathlib import Path
from sys import argv

source=Path(argv[1]).read_text()
start=source.index('  const bool on=',source.index(' template<class View,class Sink>bool process('))
end=source.index('    off_pending(sink);\n    if(on&&held.count){',start)
source=(source[:start]+'''  const bool on=p[arpIsOn]==1.f;
  const int octaves=static_cast<int>(p[arpOctave])+1;
  const auto pulse=clock.sample(on,held.count,last_count||initial_zero_count);
  if(pulse.reset){step=0;off_pending(sink);sink.beat();}
  if(pulse.trigger){
   {
'''+source[end:])
start=source.index('#if defined(S1_NO_DOUBLE)',source.index('     if(length){',start))
end=source.index('      step=beat;',start)
source=source[:start]+'      const int beat=pulse.index;\n'+source[end:]
source=source.replace('const View &held,Sink &sink){','const View &held,Sink &sink, fenced_s1_clock::Clock &clock){',1)
source=source.replace('namespace s1source_sequence {','#include "clock.hpp"\nnamespace fenced_event_adapter {\nusing namespace s1source_sequence;')
source=source.replace('if(!can_process(p,n,held))return false;','if(!tuning_valid(p,n,notes_per_octave())||clock.rate()!=p[arpRate]||clock.multiplier()!=p[arpSeqTempoMultiplier])return false;')
Path(argv[2]).write_text(source)
