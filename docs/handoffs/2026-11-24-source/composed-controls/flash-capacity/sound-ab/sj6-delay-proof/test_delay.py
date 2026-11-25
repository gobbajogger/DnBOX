"""Independent impulse contracts read from Build9's original StereoDelay render."""
import sys, unittest
import numpy as np
if '--old' in sys.argv:
    i=sys.argv.index('--old')
    sys.path.insert(0,sys.argv[i+1])
    del sys.argv[i:i+2]
    from fx import delay
else:
    from sj6_stereo_delay import delay

class Controls:
    def __init__(self, **props): self.props=props

class Delay(unittest.TestCase):
    def test_crossfade_and_dual_mono(self):
        x=np.zeros((2,32));x[:,0]=[1,.25]
        y=delay(x,Controls(dryWetMix=.75,feedback=.5,time=4/48000),True)
        want=np.zeros_like(x)
        want[:,0]=[.75,.1875]
        for i in range(1,8): want[:,4*i]=x[:,0]*.25*.5**(i-1)
        np.testing.assert_array_equal(y,want)

    def test_ping_pong_feedback_traverses_both_lines(self):
        x=np.zeros((2,20));x[0,0]=1
        y=delay(x,Controls(dryWetMix=0,feedback=.0625,time=4/48000,pingPong=True),True)
        want=np.zeros_like(x)
        want[0,[4,13]]=[.5,.015625]
        want[1,[8,17]]=[.25,.0078125]
        np.testing.assert_array_equal(y,want)

    def test_fractional_interpolation(self):
        x=np.zeros((2,20));x[0,0]=1
        y=delay(x,Controls(dryWetMix=0,feedback=0,time=4.5/48000),True)
        want=np.zeros_like(x);want[0,[4,5]]=.5
        np.testing.assert_array_equal(y,want)

    def test_bypass_is_dry_even_with_nonzero_mix(self):
        x=np.arange(64,dtype='float64').reshape(2,32)/64
        np.testing.assert_array_equal(delay(x,Controls(dryWetMix=.9),False),x)

if __name__=='__main__':unittest.main()
