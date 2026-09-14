import unittest
import subprocess
from pathlib import Path
import sys


BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "./build/main").resolve()


class TwoQueueCacheTests(unittest.TestCase):
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
            (2, [5, 5, 5, 5, 5], 4),
            (2, [1, 2, 1, 2, 1, 2], 4),
            (3, [1, 2, 3, 4, 1, 2, 1, 2], 2),
            (4, [1, 2, 3, 4, 5, 1, 6, 2, 3, 4, 3, 4], 2),
            (4, [1, 2, 3, 4, 5, 6, 7, 1, 1], 1),
            (6, [1, 2, 3, 4, 5, 6, 100, 200, 7, 8, 9, 10, 11, 12, 100, 200, 1000, 1001, 1002, 1003, 
                 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 
                 1018, 1019, 1020, 1021, 1022, 1023, 1024, 100, 200], 2),
            (6, [1, 101, 2, 102, 3, 103, 1, 104, 2, 105, 3, 106, 1, 107, 2, 108, 3, 109, 1, 110, 2,
                 111, 3, 112, 1, 113, 2, 114, 3, 115, 1, 116, 2, 117, 3, 118, 1, 119, 2, 120, 3, 
                 121, 1, 122, 2, 123, 3, 124, 1, 125, 2, 126, 3, 127, 1, 128, 2, 129, 3, 130], 24)
        ]
        for capacity, values, expected in cases:
            with self.subTest(capacity=capacity, values=values):
                self.check_hits(capacity, values, expected)


if __name__ == "__main__":
    unittest.main()