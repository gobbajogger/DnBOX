import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from replay import validate_seed


class FlashSeedTest(unittest.TestCase):
    def test_checks_code_and_allows_saved_user_state(self):
        protected = bytes([0xa5]) * 0x9d000
        with tempfile.TemporaryDirectory() as directory:
            package = Path(directory) / "candidate.fwsc"
            flash = Path(directory) / "flash.bin"
            package.write_bytes(b"candidate")
            seed = bytearray(protected + bytes(0x100000 - len(protected)))
            seed[0x9d000] = 42
            with patch("replay.package_flash_image", return_value=protected):
                flash.write_bytes(seed)
                validate_seed(package, flash)
                seed[0x9cfff] ^= 1
                flash.write_bytes(seed)
                with self.assertRaisesRegex(ValueError, "firmware bytes"):
                    validate_seed(package, flash)


if __name__ == "__main__":
    unittest.main()
