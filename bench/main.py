import argparse
from benchmark import Benchmarker
from generator import GENERATOR_PATTERNS
from capacity_policy import CapacitySharingPolicy
import nhwc


MAX_CACHE_LEVELS = 5
DEFAULT_CACHE_SIZE = 6510
DEFAULT_REQUESTS_COUNT = 131072
DEFAULT_KEY_COUNT = 65536
DEFAULT_SEED = 42


def main():
    parser = argparse.ArgumentParser(description="Generate cache benchmark workloads.")
    parser.add_argument("-s", "--cache-size", type=int, default=DEFAULT_CACHE_SIZE)
    parser.add_argument("-r", "--requests", type=int, default=DEFAULT_REQUESTS_COUNT)
    parser.add_argument("-k", "--keys", type=int, default=DEFAULT_KEY_COUNT)
    parser.add_argument("-g", "--seed", type=int, default=DEFAULT_SEED)
    parser.add_argument("-p", "--pattern", choices=["all", *nhwc.SUPPORTED_CACHE_ALGORITHMS],
                        default="all")

    args = parser.parse_args()

    if args.cache_size <= 0:
        raise ValueError("cache-size must be > 0")

    if args.requests <= 0:
        raise ValueError("requests must be > 0")

    if args.keys <= 0:
        raise ValueError("keys must be > 0")

    benchmarker = Benchmarker(MAX_CACHE_LEVELS, nhwc.SUPPORTED_CACHE_ALGORITHMS,
                              list(CapacitySharingPolicy), GENERATOR_PATTERNS)
    result = benchmarker.run(args.cache_size, args.requests, args.keys, args.seed)
    print(result)
    benchmarker.save_report(benchmarker.analyze(result), "report.md")


if __name__ == "__main__":
    main()
