import subprocess


SUPPORTED_CACHE_ALGORITHMS = ["ARC", "LFU", "LRU", "2Q", "LIRS"]
CONFIG_FILENAME = "config.txt"
MAIN_BINARY_PATH = "../build/main"
IDEAL_BINARY_PATH = "../build/ideal_cache"


def dump_nhwc_config(algorithms):
    with open(CONFIG_FILENAME, "w") as config_file:
        print(len(algorithms), " ".join(algorithms), file=config_file)


def run_cache_binary(path, data):
    return int(subprocess.run([path], input=data, text=True, capture_output=True).stdout)


def run_nhwc_cache(capacities, algorithms, values):
    dump_nhwc_config(algorithms)
    data = " ".join(map(str, capacities)) + f" {len(values)}\n" + " ".join(map(str, values))
    return run_cache_binary(MAIN_BINARY_PATH, data)


def run_ideal_cache(capacity, values):
    data = f"{capacity} {len(values)}\n" + " ".join(map(str, values))
    return run_cache_binary(IDEAL_BINARY_PATH, data)
