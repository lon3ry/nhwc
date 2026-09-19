#!/usr/bin/env -S uv run --script
# /// script
# dependencies = [
#   "numpy",
#   "matplotlib"
# ]
# ///
import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter
import numpy as np
import subprocess
from pathlib import Path
import sys
import os

from itertools import combinations


OUTPUT_DIRECTORY = "res"
DATA_DIRECTORY = "data"
NHWC_CONFIG_FILENAME = "config.txt"
BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "../build/main").resolve()
CACHES = ["ARC", "LFU", "LRU", "2Q", "LIRS"]


if not BINARY.exists():
    print("error: the 'main' binary doesn't exists")
    exit(1)

directory = Path(DATA_DIRECTORY)
if not directory.exists():
    print("error: the 'data' directory doesn't exists")
    exit(1)

for file in directory.iterdir():
    if not file.is_file():
        continue

    fig, ax = plt.subplots()
    ax.set_xlabel("Cache Size")
    ax.set_ylabel("Hit Ratio (%)")
    ax.grid()
    ax.yaxis.set_major_formatter(PercentFormatter(xmax=1.0, symbol=""))

    with open(file) as f:
        n = int(f.readline())
        ax.set_title(f"{file.name} ({n} requests)")

        values = [int(f.readline()) for i in range(n)]

        for combination in combinations(CACHES, 2):
            with open(NHWC_CONFIG_FILENAME, "w") as config_file:
                print(len(combination), " ".join(combination), file=config_file)

            x = []
            y = []
            for capacity in range(2, 2**5 + 1):
                data = f"{capacity} " * len(combination) + f"{len(values)}\n" + \
                       " ".join(map(str, values))
                result = subprocess.run(
                    [BINARY], input=data, text=True, capture_output=True, timeout=5
                )
                n = int(result.stdout)
                x.append(capacity * len(combination))
                y.append(n / len(values))
            ax.plot(x, y, label=f" + ".join(combination))

        ax.legend()
        filename = file.stem + ".png"
        plt.savefig(filename)
        plt.close()
        print(f"[+] {filename}")
