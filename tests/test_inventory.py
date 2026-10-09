import tempfile
import unittest
from pathlib import Path
from tools.inventory import detect_archive, inventory


class InventoryTests(unittest.TestCase):
    def test_rar4(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td) / "a.rar"
            p.write_bytes(bytes.fromhex("526172211a0700") + b"data")
            self.assertEqual(detect_archive(p), "RAR4")

    def test_directory_inventory(self):
        with tempfile.TemporaryDirectory() as td:
            p = Path(td)
            (p / "pak0.pk3").write_bytes(b"123")
            (p / "readme.txt").write_text("hi")
            result = inventory(p)
            self.assertEqual(result["total_files"], 2)
            self.assertEqual(result["extensions"][".pk3"], 1)
            self.assertEqual(result["game_packages"][0]["name"], "pak0.pk3")


if __name__ == "__main__":
    unittest.main()
