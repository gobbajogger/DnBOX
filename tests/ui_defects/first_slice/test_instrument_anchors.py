import unittest
from instrument import active_bank_anchor

class ActualPanelAnchorTests(unittest.TestCase):
    def test_historical_and_r22_radius(self):
        for radius in (4,9):
            anchor='rbox(6, ay, 228, ah, %d, RGB(36,48,54));'%radius
            self.assertEqual(active_bank_anchor(anchor),anchor)
    def test_missing_unknown_and_duplicated_paint_fail_closed(self):
        a='rbox(6, ay, 228, ah, 4, RGB(36,48,54));'
        b='rbox(6, ay, 228, ah, 9, RGB(36,48,54));'
        for text in ('',a.replace(', 4,',', 5,'),a+a,a+b):
            with self.assertRaises(ValueError): active_bank_anchor(text)
