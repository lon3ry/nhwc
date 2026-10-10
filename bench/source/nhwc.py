import subprocess
import tempfile
import os


SUPPORTED_CACHE_ALGORITHMS = ["ARC", "LFU", "LRU", "2Q", "LIRS"]
MAIN_BINARY_PATH = "../../build/nhwc"
IDEAL_BINARY_PATH = "../../build/ideal_cache"


def run_cache_binary(path, data, config_path=None):
    cmd = [path]
    if config_path is not None:
        cmd += ["--config", config_path]
    proc = subprocess.run(cmd, input=data, text=True, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError(f"{path} failed: {proc.stderr}")
    return int(proc.stdout)


def run_nhwc_cache(capacities, values, config_path):
    data = " ".join(map(str, capacities)) + f" {len(values)}\n" + " ".join(map(str, values))
    return run_cache_binary(MAIN_BINARY_PATH, data, config_path)


def run_ideal_cache(capacity, values):
    data = f"{capacity} {len(values)}\n" + " ".join(map(str, values))
    return run_cache_binary(IDEAL_BINARY_PATH, data)


def write_nhwc_config(algorithms, config_path):
    with open(config_path, "w") as f:
        print(len(algorithms), " ".join(algorithms), file=f)
