# SPDX-License-Identifier: GPL-3.0-only
"""Regression: repository renames must not change TouchOSC control identities."""
import importlib.util
from pathlib import Path
import unittest
import uuid
from xml.dom import minidom
import zlib

ROOT = Path(__file__).resolve().parents[2]
LAYOUT = ROOT / "web/offline-sequencer/touchosc"
spec = importlib.util.spec_from_file_location("touchosc_pack", LAYOUT / "pack.py")
pack = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pack)


class TouchOSCIdentityTest(unittest.TestCase):
    def test_all_control_ids_keep_original_namespace(self):
        xml = pack.make_xml(LAYOUT)
        nodes = minidom.parseString(xml).getElementsByTagName("node")
        self.assertEqual(len(nodes), 47)
        ids = []
        for node in nodes:
            properties = node.getElementsByTagName("properties")[0]
            name = next(
                prop.getElementsByTagName("value")[0].firstChild.data
                for prop in properties.getElementsByTagName("property")
                if prop.getElementsByTagName("key")[0].firstChild.data == "name"
            )
            expected = str(uuid.uuid5(uuid.NAMESPACE_URL,
                "https://github.com/gobbajogger/X0X37-for-Loopy-Pro/touchosc/" + name))
            self.assertEqual(node.getAttribute("ID"), expected, name)
            ids.append(expected)
        self.assertEqual(len(set(ids)), len(ids))
        self.assertEqual(zlib.decompress(pack.make_tosc(LAYOUT)), xml)
        self.assertEqual(pack.make_xml(LAYOUT), xml)


if __name__ == "__main__":
    unittest.main()
