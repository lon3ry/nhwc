import pandas as pd
import nhwc
from generator import Generator
from itertools import permutations
from capacity_policy import calc_cache_levels_capacities, CapacitySharingPolicy


def dict_sorted_by_value(dictionary):
    return dict(sorted(dictionary.items(), key=lambda item: item[1], reverse=True))



class Benchmarker:
    def __init__(self, max_cache_levels, cache_algorithms, capacity_sharing_policies,
                 generator_patterns):
        if max_cache_levels <= 0:
            raise ValueError("max_cache_levels must be > 0")

        if any([policy not in list(CapacitySharingPolicy) for policy in capacity_sharing_policies]):
            raise ValueError("unknown cache policy")

        if any([algorithm not in nhwc.SUPPORTED_CACHE_ALGORITHMS for algorithm in cache_algorithms]):
            raise ValueError("unknown cache algorithm")

        self.max_cache_levels = max_cache_levels
        self.cache_algorithms = cache_algorithms
        self.generator_patterns = generator_patterns
        self.capacity_sharing_policies = capacity_sharing_policies

    def run(self, cache_size, requests_count, key_count, seed, run_ideal=True):
        generator = Generator(seed, requests_count, key_count)

        result = {}
        for policy in self.capacity_sharing_policies:
            result[policy] = {}
            for pattern in self.generator_patterns:
                data = generator.generate(pattern, cache_size)

                result[policy][pattern] = {}
                for cache_levels in range(1, self.max_cache_levels + 1):
                    levels_capacities = calc_cache_levels_capacities(cache_size, cache_levels,
                                                                     policy)
                    for algorithms in permutations(self.cache_algorithms, cache_levels):
                        if run_ideal:
                            run_result = nhwc.run_ideal_cache(cache_size, data)
                            result[policy][pattern]["Ideal Cache"] = run_result / requests_count

                        run_result = nhwc.run_nhwc_cache(levels_capacities, algorithms, data)
                        result[policy][pattern][" + ".join(algorithms)] = run_result / requests_count
                    print(f"[+] {pattern} with {requests_count} requests, {cache_size} capacity "
                          f"({", ".join(map(str, levels_capacities))})")

        return result

    def analyze(self, data):
        best = {}
        for policy in data.keys():
            best[policy] = {
                name: dict([
                    max(
                        (
                            (key, [value, metrics["Ideal Cache"] - value])
                            for key, value in metrics.items()
                            if key != "Ideal Cache"
                        ),
                        key=lambda item: item[1][0]
                    )
                ])
                for name, metrics in data[policy].items()
            }
        return best

    def save_report(self, data, filename):
        for policy in data.keys():
            rows = []
            for pattern, algos in data[policy].items():
                for algo, (ratio, diff) in algos.items():
                    rows.append({
                        "Pattern": pattern,
                        "Best Algorithm": algo,
                        "Hit Ratio (%)": ratio * 100,
                        "Difference with Ideal (%)": diff * 100,
                    })

            df = pd.DataFrame(rows)
            print(df.to_markdown(index=False))
            print()
