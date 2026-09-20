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
from itertools import permutations

plt.rcParams['axes.prop_cycle'] = (
    plt.cycler(color=plt.rcParams['axes.prop_cycle'].by_key()['color'])
    * plt.cycler(linestyle=['-', '--', '-.', ':'])
)

OUTPUT_DIRECTORY = "res"
DATA_DIRECTORY = "data"
CACHES = ["ARC", "LFU", "LRU", "2Q", "LIRS"]
NHWC_CONFIG_FILENAME = "config.txt"
NHWC_CACHE_LEVELS = 2
MINIMAL_CACHE_LEVEL_CAPACITY = 2
MAXIMUM_CACHE_LEVEL_CAPACITY = 2 ** 5
NHWC_BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "../build/main").resolve()
IDEAL_CACHE_BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 2
                     else "../build/ideal_cache").resolve()


def run_ideal_cache(capacity, values):
    data = f"{capacity} {len(values)}\n" + " ".join(map(str, values))
    result = subprocess.run(
        [IDEAL_CACHE_BINARY], input=data, text=True, capture_output=True, timeout=5
    )
    return int(result.stdout)


def benchmark_ideal_cache(capacity_min, capacity_max, values):
    x = []
    y = []
    for capacity in range(capacity_min, capacity_max + 1):
        x.append(capacity * NHWC_CACHE_LEVELS)
        result = run_ideal_cache(capacity * NHWC_CACHE_LEVELS, values)
        y.append(result / len(values))
    return (x, y)


def run_multi_level_cache(capacities, values):
    data = " ".join(map(str, capacities)) + f" {len(values)}\n" + " ".join(map(str, values))
    result = subprocess.run(
        [NHWC_BINARY], input=data, text=True, capture_output=True, timeout=5
    )
    return int(result.stdout)


def dump_nhwc_config(algorithms):
    with open(NHWC_CONFIG_FILENAME, "w") as config_file:
        print(len(algorithms), " ".join(algorithms), file=config_file)


def benchmark_nhwc(capacity_min, capacity_max, values):
    data = {}
    for permutation in permutations(CACHES, NHWC_CACHE_LEVELS):
        dump_nhwc_config(permutation)
        x = []
        y = []
        for capacity in range(capacity_min, capacity_max + 1):
            x.append(capacity * len(permutation))
            result = run_multi_level_cache([capacity] * len(permutation), values)
            y.append(result / len(values))
        data[" + ".join(permutation)] = (x, y)
    return data


if not NHWC_BINARY.exists():
    print("error: the 'main' binary doesn't exists")
    exit(1)

directory = Path(DATA_DIRECTORY)
if not directory.exists():
    print("error: the 'data' directory doesn't exists")
    exit(1)

for file in directory.iterdir():
    if not file.is_file():
        continue

    fig, ax = plt.subplots(figsize=(15, 10), constrained_layout=True)
    ax.set_xlabel("Cache Size")
    ax.set_ylabel("Hit Ratio (%)")
    ax.minorticks_on()
    ax.grid(which="both")
    ax.yaxis.set_major_formatter(PercentFormatter(xmax=1.0, symbol=""))

    with open(file) as f:
        n = int(f.readline())
        ax.set_title(f"{file.name} ({n} requests)")

        values = [int(f.readline()) for i in range(n)]

        x, y = benchmark_ideal_cache(MINIMAL_CACHE_LEVEL_CAPACITY, MAXIMUM_CACHE_LEVEL_CAPACITY,
                                     values)
        ax.plot(x, y, label="Ideal Cache")

        data = benchmark_nhwc(MINIMAL_CACHE_LEVEL_CAPACITY, MAXIMUM_CACHE_LEVEL_CAPACITY, values)
        for algorithms in data.keys():
            x, y = data[algorithms]
            ax.plot(x, y, label=algorithms)

        ax.legend()
        filename = file.stem + ".png"
        plt.savefig(filename)
        plt.close()

        print(f"[+] {filename}")
