import copy
from decimal import Decimal as D
import unittest
import projection

def preset():
    p={"uid":"synthetic", "version":"1.7", "unknown":{"keep":[1,2,3]}, "parameterValues":[]}
    vals={"neonattack":D(".2"),"neondecay":D(".3"),"neonsustain":D(".4"),"neonrelease":D(".5"),
    "neonattack2":D(".7"),"neondecay2":D(".6"),"neonsustain2":D(".8"),"neonrelease2":D(".9"),
    "neonfilterAttack":D(".1"),"neonfilterDecay":D(".2"),"neonfilterSustain":D(".3"),"neonfilterRelease":D(".4"),
    "neonfilterCutoff":D(".4"),"neoncutoffLayer1":D(".8"),"neonfilterRez":D(".1"),"neonresonanceLayer1":D(".2"),
    "neonsawVolume":D("1.001"),"neonpulseVolume":D("1.001"),"neonsubVolume":D(".2"),"neonnoiseVolume":D(".1"),
    "neonvolumeControl":D("1.32"),"neonfilterStrength":D(".2"),"neonadsrLinkEnable":1,"future/~thing":D("-.008403362")}
    p["parameterValues"]=[{"identifier":k,"value":v} for k,v in vals.items()]
    return p

class ProjectionTests(unittest.TestCase):
    def call(self,p,**kw):
        return projection.project_preset(p, acknowledge_approximation=True,cutoff_source="neoncutoffLayer1",resonance_source="neonresonanceLayer1",**kw)
    def test_original_values_preserved_and_three_envs_distinct(self):
        p=preset(); before=copy.deepcopy(p); q=self.call(p)
        self.assertEqual(p,before); self.assertEqual(q["source"],p)
        self.assertEqual(q["status"],"APPROXIMATE_NOT_J6_DSP")
        self.assertNotEqual(q["engine"]["envelopes"][0],q["engine"]["envelopes"][1])
        self.assertEqual(q["engine"]["saw"],1.001);self.assertEqual(q["engine"]["level"],1.32)
        self.assertIn("future/~thing",q["unsupported"])
        self.assertIn("neonadsrLinkEnable",q["unsupported"])
        self.assertEqual(q["engine"]["lfo"],[{"hz":0.0,"depth":0.0,"routes":0}]*3)
    def test_no_implicit_ack_or_cutoff_priority(self):
        p=preset()
        for kw in ({},{"acknowledge_approximation":True},{"acknowledge_approximation":True,"cutoff_source":"neonfilterCutoff"}):
            with self.assertRaises(ValueError): projection.project_preset(p,**kw)
        a=self.call(p);b=projection.project_preset(p,acknowledge_approximation=True,cutoff_source="neonfilterCutoff",resonance_source="neonfilterRez")
        self.assertNotEqual(a["engine"]["cutoff_hz"],b["engine"]["cutoff_hz"])
        self.assertIn("neonfilterCutoff",a["unsupported"])
    def test_rejects_bad_mapped_values_not_clamp(self):
        for value in (D("1.002"),D("-0.01"),D("Infinity"),True,0.3):
            p=preset(); next(x for x in p["parameterValues"] if x["identifier"]=="neonsawVolume")["value"]=value
            with self.subTest(value=value),self.assertRaises(ValueError): self.call(p)
    def test_rejects_duplicate_missing_and_future_version(self):
        for kind in ("duplicate","missing","future"):
            p=preset()
            if kind=="duplicate":p["parameterValues"].append(dict(p["parameterValues"][0]))
            elif kind=="missing":p["parameterValues"].pop(0)
            else:p["version"]="future"
            with self.subTest(kind=kind),self.assertRaises(ValueError):self.call(p)
    def test_unhashable_policy_or_version_is_value_error(self):
        for field in ("version","cutoff_source","resonance_source"):
            for value in ([],{},None,True):
                p=preset();kw=dict(acknowledge_approximation=True,cutoff_source="neoncutoffLayer1",resonance_source="neonresonanceLayer1")
                if field=="version":p[field]=value
                else:kw[field]=value
                with self.subTest(field=field,value=value),self.assertRaises(ValueError):projection.project_preset(p,**kw)
    def test_source_is_detached(self):
        p=preset();q=self.call(p);p["unknown"]["keep"].append(4)
        self.assertEqual(q["source"]["unknown"]["keep"],[1,2,3])

if __name__=="__main__":unittest.main()
