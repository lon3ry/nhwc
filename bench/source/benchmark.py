import os
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed
from functools import partial
from itertools import permutations

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

import pandas as pd

import nhwc
from capacity_policy import calc_cache_levels_capacities, CapacitySharingPolicy
from generator import Generator


MAX_WINNERS_TO_DISPLAY = 10


def dict_sorted_by_value(dictionary):
    return dict(sorted(dictionary.items(), key=lambda item: item[1], reverse=True))


class Benchmarker:
    def __init__(self, max_cache_levels, cache_algorithms, capacity_sharing_policies,
                 generator_patterns):
        if max_cache_levels <= 0:
            raise ValueError("max_cache_levels must be > 0")

        if any(policy not in list(CapacitySharingPolicy) for policy in capacity_sharing_policies):
            raise ValueError("unknown cache policy")

        if any(algorithm not in nhwc.SUPPORTED_CACHE_ALGORITHMS for algorithm in cache_algorithms):
            raise ValueError("unknown cache algorithm")

        self.max_cache_levels = max_cache_levels
        self.cache_algorithms = cache_algorithms
        self.generator_patterns = generator_patterns
        self.capacity_sharing_policies = capacity_sharing_policies

    def generate_workloads(self, cache_size, requests_count, key_count, seed):
        generator = Generator(seed, requests_count, key_count)

        return {
            pattern: generator.generate(pattern, cache_size)
            for pattern in self.generator_patterns
        }

    def _run_nhwc(self, capacities, algorithms, data):
        fd, config_path = tempfile.mkstemp(suffix=".txt", prefix="nhwc_cfg_")
        try:
            with os.fdopen(fd, "w") as f:
                f.write(f"{len(algorithms)} {' '.join(algorithms)}\n")

            return nhwc.run_nhwc_cache(capacities, data, config_path)
        finally:
            try:
                os.unlink(config_path)
            except OSError:
                pass

    def _build_tasks(self, workloads, cache_size, run_ideal):
        tasks = []

        for pattern, data in workloads.items():
            for policy in self.capacity_sharing_policies:
                if run_ideal:
                    tasks.append((
                        policy,
                        pattern,
                        "Ideal Cache",
                        partial(nhwc.run_ideal_cache, cache_size, data),
                    ))

                for cache_levels in range(1, self.max_cache_levels + 1):
                    capacities = calc_cache_levels_capacities(cache_size, cache_levels, policy)

                    for algorithms in permutations(self.cache_algorithms, cache_levels):
                        label = " + ".join(algorithms)
                        tasks.append((
                            policy,
                            pattern,
                            label,
                            partial(
                                self._run_nhwc,
                                capacities,
                                algorithms,
                                data,
                            ),
                        ))

        return tasks

    def run(self, cache_size, requests_count, key_count, seed,
            run_ideal=True, max_workers=None):
        workloads = self.generate_workloads(cache_size, requests_count, key_count, seed)
        tasks = self._build_tasks(workloads, cache_size, run_ideal)

        result = {
            policy: {pattern: {} for pattern in self.generator_patterns}
            for policy in self.capacity_sharing_policies
        }

        if max_workers is None:
            max_workers = os.cpu_count() or 4

        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            future_to_key = {
                executor.submit(fn): (policy, pattern, label)
                for policy, pattern, label, fn in tasks
            }

            for future in as_completed(future_to_key):
                policy, pattern, label = future_to_key[future]

                try:
                    hits = future.result()
                except Exception as e:
                    print(f"[!] {policy}, {pattern}, {label} failed: {e}")
                    continue

                result[policy][pattern][label] = hits / requests_count
                print(f"[+] {policy}, {pattern}, {label}")

        return {
            "metadata": {
                "cache_size": cache_size,
                "requests_count": requests_count,
                "key_count": key_count,
                "seed": seed,
            },
            "results": result,
        }

    def analyze(self, data):
        best = {}

        for policy, patterns in data["results"].items():
            best[policy] = {}

            for pattern, metrics in patterns.items():
                ideal = metrics.get("Ideal Cache")
                configs = {
                    k: v for k, v in metrics.items()
                    if k != "Ideal Cache"
                }

                single = {
                    k: v for k, v in configs.items()
                    if " + " not in k
                }
                multi = {
                    k: v for k, v in configs.items()
                    if " + " in k
                }

                def pick(d):
                    if not d:
                        return None

                    max_ratio = max(d.values())
                    winners = sorted(
                        k for k, v in d.items()
                        if v == max_ratio
                    )

                    if len(winners) < MAX_WINNERS_TO_DISPLAY:
                        display = ", ".join(winners)
                    else:
                        display = len(winners)

                    return display, max_ratio, len(winners)

                best_single = pick(single)
                best_multi = pick(multi)

                candidates = [
                    c for c in (best_single, best_multi)
                    if c is not None
                ]

                best_configuration = (
                    max(candidates, key=lambda c: c[1])
                    if candidates else None
                )

                best[policy][pattern] = {
                    "Best Single": best_single,
                    "Best Multi": best_multi,
                    "Best Configuration": best_configuration,
                    "Ideal": ideal,
                }

        return {
            "metadata": data["metadata"],
            "results": best,
        }

    def save_report(self, data, filename):
        chunks = []

        for policy, patterns in data["results"].items():
            rows = []

            for pattern, info in patterns.items():
                ideal = info["Ideal"]

                def unpack(entry):
                    if entry is None:
                        return "-", None, None

                    display, ratio, _ = entry
                    diff = (
                        (ideal - ratio) * 100
                        if ideal is not None else None
                    )
                    return display, ratio * 100, diff

                single_display, single_ratio, single_diff = unpack(
                    info["Best Single"]
                )
                multi_display, multi_ratio, multi_diff = unpack(
                    info["Best Multi"]
                )
                overall_display, overall_ratio, overall_diff = unpack(
                    info["Best Configuration"]
                )

                rows.append({
                    "Pattern": pattern,
                    "Best Configuration": overall_display,
                    "Best Hit (%)": overall_ratio,
                    "Best Diff Ideal (%)": overall_diff,
                    "Single Algorithm": single_display,
                    "Single Hit (%)": single_ratio,
                    "Single Diff Ideal (%)": single_diff,
                    "Multi Algorithm": multi_display,
                    "Multi Hit (%)": multi_ratio,
                    "Multi Diff Ideal (%)": multi_diff,
                })

            df = pd.DataFrame(rows)
            table = df.to_markdown(index=False, floatfmt=".2f")
            chunks.append(f"# Policy: {policy}\n\n{table}\n\n")

        with open(filename, "w") as f:
            f.write("".join(chunks))

    def save_hist(self, data, filename_prefix, top_n=5):
        metadata = data["metadata"]
        results = data["results"]

        requests_count = metadata["requests_count"]
        key_count = metadata["key_count"]

        first_policy = next(iter(results.values()), {})
        first_pattern = next(iter(first_policy.values()), {})

        if (
            "Best Single" in first_pattern
            or "Best Configuration" in first_pattern
        ):
            raise ValueError(
                "save_hist expects the raw results from run(), "
                "not the output of analyze()"
            )

        level_colors = plt.cm.tab10.colors

        for policy, patterns in results.items():
            n_patterns = len(patterns)
            if n_patterns == 0:
                continue

            cols = min(n_patterns, 3)
            rows = (n_patterns + cols - 1) // cols

            fig, axes = plt.subplots(
                rows,
                cols,
                figsize=(6 * cols, 4.5 * rows),
                squeeze=False,
            )

            used_levels = set()

            for ax, (pattern, metrics) in zip(axes.flat, patterns.items()):
                ideal = metrics.get("Ideal Cache")
                configs = {
                    k: v for k, v in metrics.items()
                    if k != "Ideal Cache"
                }

                # Top N by hit ratio; ties broken alphabetically.
                top = sorted(
                    configs.items(),
                    key=lambda kv: (-kv[1], kv[0]),
                )[:top_n]

                labels = [label for label, _ in top]
                values = [value * 100 for _, value in top]
                levels = [label.count(" + ") + 1 for label in labels]

                used_levels.update(levels)

                colors = [
                    level_colors[(lvl - 1) % len(level_colors)]
                    for lvl in levels
                ]

                bars = ax.bar(range(len(top)), values, color=colors)
                ax.bar_label(bars, fmt="%.2f", fontsize=8, padding=2)

                if ideal is not None:
                    ax.axhline(
                        ideal * 100,
                        color="black",
                        linestyle="--",
                        linewidth=1,
                    )

                ax.set_title(str(pattern))
                ax.set_ylabel("Hit ratio (%)")
                ax.set_xticks(range(len(top)))
                ax.set_xticklabels(
                    [label.replace(" + ", "\n+ ") for label in labels],
                    rotation=0,
                    ha="center",
                    fontsize=8,
                )

                if values:
                    lo, hi = min(values), max(values)

                    if ideal is not None:
                        hi = max(hi, ideal * 100)

                    margin = max((hi - lo) * 0.3, 1.0)
                    ax.set_ylim(
                        max(0, lo - margin),
                        min(100, hi + margin),
                    )

            # Hide unused subplots.
            for ax in list(axes.flat)[n_patterns:]:
                ax.set_visible(False)

            legend_items = [
                Patch(
                    facecolor=level_colors[(lvl - 1) % len(level_colors)],
                    label=f"{lvl} level{'s' if lvl > 1 else ''}",
                )
                for lvl in sorted(used_levels)
            ]

            if any(
                metrics.get("Ideal Cache") is not None
                for metrics in patterns.values()
            ):
                legend_items.append(
                    plt.Line2D(
                        [0],
                        [0],
                        color="black",
                        linestyle="--",
                        label="Ideal Cache",
                    )
                )

            fig.legend(handles=legend_items, loc="upper right", fontsize=16)

            policy_name = getattr(policy, "name", str(policy)).lower()

            fig.suptitle(
                f"Top {top_n} configurations per pattern with {policy_name} policy, {key_count} "
                f"keys, {requests_count} requests",
                fontsize=18,
            )

            fig.tight_layout(rect=(0, 0, 1, 0.95))

            out_path = f"{filename_prefix}_{policy_name}.png"
            fig.savefig(out_path, dpi=150)
            plt.close(fig)
