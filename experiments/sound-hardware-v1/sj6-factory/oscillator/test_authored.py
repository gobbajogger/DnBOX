import struct
import unittest

from replay import authored_layers


class AuthoredPwmTests(unittest.TestCase):
    def test_first_match_presence_and_main_pass_defaults(self):
        keys = {n: str(n) for n in (78, 79, 82, 83, 210, 211)}
        parameters = [(82, 1.), (83, 1.), (78, -.0), (78, .75), (211, .0)]
        record = {"uid": "synthetic", "parameterValues": [
            {"identifier": keys[key], "value": value} for key, value in parameters]}
        first, second = list(authored_layers(record, keys))
        for row, expected in ((first, "00000080"), (second, "00000000")):
            self.assertEqual(struct.pack("<f", row[3]).hex(), expected)
            self.assertEqual(struct.pack("<f", row[4]).hex(), "00000000")
        self.assertEqual(struct.pack("<f", first[5]["authored_pwm_amount"]).hex(), "00000080")
        self.assertIsNone(first[5]["authored_pwm_enable"])
        self.assertIsNone(second[5]["authored_pwm_amount"])
        self.assertEqual(second[5]["authored_pwm_enable"], .0)


if __name__ == "__main__":
    unittest.main()
