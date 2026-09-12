import unittest
import subprocess
from pathlib import Path
import sys


BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "./build/main").resolve()


class LRUCacheTests(unittest.TestCase):
    def run_binary(self, data):
        print(str(BINARY))
        return subprocess.run(
            [str(BINARY)], input=data, text=True, capture_output=True, timeout=5
        )

    def check_hits(self, capacity, values, expected):
        data = f"{capacity} {len(values)}\n" + " ".join(map(str, values))
        result = self.run_binary(data)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "")
        self.assertEqual(result.stdout, f"{expected}\n", data)

    def test_hit_count(self):
        cases = [
            (2, [6, 1, 2, 1, 2, 1, 2], 4)
        ]
        for capacity, values, expected in cases:
            with self.subTest(capacity=capacity, values=values):
                self.check_hits(capacity, values, expected)


if __name__ == "__main__":
    unittest.main()