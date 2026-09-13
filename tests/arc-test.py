import unittest
import subprocess
from pathlib import Path
import sys


BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "./build/main").resolve()


class ARCCacheTests(unittest.TestCase):
    def run_binary(self, data):
        print(str(BINARY))
        return subprocess.run(
            [str(BINARY)], input=data, text=True, capture_output=True, timeout=5
        )

    def check_hits(self, capacity, values, expected):
        data = f"{capacity} {len(values)}\n" + " ".join(map(str, values))
        result = self.run_binary(data)
        # self.assertEqual(result.returncode, 0, result.stderr)
        # self.assertEqual(result.stderr, "")
        self.assertEqual(result.stdout, f"{expected}\n", data)

    def test_hit_count(self):
        cases = [
            (1, [1, 2, 1, 2, 1], 0),
            (2, [1, 2, 1, 2, 1, 2], 4),
            (3, [1, 2, 3, 4, 5, 6], 0),
            (2, [1, 1, 2, 2, 3, 3], 3),
            (2, [1, 2, 1, 2, 1, 2], 4),
            (2, [1, 2, 3, 1, 2, 3, 1, 2, 3], 0),
            (3, [1, 1, 1, 1, 1, 1, 1, 1], 7),
            (1, [1, 2, 1, 2, 1], 0),
            (4, [1, 2, 1, 2, 10, 11, 12, 13, 14, 15, 1, 2], 4),
            (5, [1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2, 3, 13, 14, 15, 16, 17,
                18, 4, 5, 1, 1, 1], 11),
            (8, [1, 2, 6, 6, 105, 1, 2, 5, 2, 6, 2, 3, 1, 2, 3, 2, 121, 4, 3, 1, 5, 4, 3, 5, 123, 6,
                 6, 3, 154, 1, 4, 3, 3, 3, 6, 5, 5, 2, 3, 140, 2, 1, 1, 4, 2, 136, 120, 4, 6, 3, 6,
                 3, 4, 125, 2, 1, 1, 2, 4, 4], 46),
            (3, [1, 1, 2, 2, 3, 3, 4, 5, 6, 1], 3),
            (4, [1, 2, 3, 4], 0),
            (3, [10, 1, 1, 2, 2, 3, 3, 4, 5, 6, 1], 3),
            (4, [1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4], 8),
            (2, [1, 2, 1, 2, 3, 4, 1, 2, 1, 2], 5),
            (2, [1, 2, 1, 2, 1, 2, 3, 1, 2, 3], 5),
            (3, [9, 1, 1, 2, 2, 3, 3, 1, 2, 3], 6),
            (4, [1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6], 7),
            (3, [1, 2, 3, 2, 1, 4, 3, 1, 2, 4], 3),
            (2, [6, 1, 2, 3, 1, 1], 1),
            (10, [1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3], 9),
            (3, [1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6], 6),
            (3, [1, 2, 1, 2, 1, 2, 3, 4, 3, 4, 3, 4, 1, 2, 3, 4], 7),
            (2, [1, 1, 2, 3, 4, 5, 6, 7, 8, 2], 1)
        ]
        for capacity, values, expected in cases:
            with self.subTest(capacity=capacity, values=values):
                self.check_hits(capacity, values, expected)


if __name__ == "__main__":
    unittest.main()
