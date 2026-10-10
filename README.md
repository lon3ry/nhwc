# NHWC

A cache algorithms research homework from [Kostantin Vladimirov's course](https://github.com/tilir/cpp-graduate). Co-authored with [Sergey Kovalenko](https://github.com/serhiosmol) and [Dmitry Lominin](https://github.com/LomininD).

This project implements multi-level cache with various policies supported, such as:

- 2Q

- ARC

- LFU

- LIRS

- LRU

## Building

You need to have [CMake](https://cmake.org) 3.28+, compatible build tools (see [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for more information) with [modules support](https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html#generator-support) and a compiler with C++23 support installed.

## Running

Submitting config with `--config` option is required.

The config file format is:
```
<number_of_levels> <cache_policies>
```
For example:
```
2 ARC LRU
```
```
1 ARC
```
The program accepts size of cache levels, then data set size and a sequence of requests. For example:
```shell
echo 2 ARC 2Q > config.txt
echo 2 4 6 1 2 1 2 1 2 | ./build/nhwc --config config.txt
```
In this case we have a two-level cache with ARC cache (size 2) and 2Q (size 4) and a sequence with 6 requests.

## Testing

We use [GoogleTest](https://google.github.io/googletest) framework for our tests. You can run them using [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).

## Benchmarking

This project includes various benchmarks. To run them, install [uv](https://docs.astral.sh/uv) and run the following:
```shell
cd bench
uv run main.py
```
This will run the benchmark with default configuration. See `--help` for all available options.


There are various data patterns supported:

| Pattern                  | Description                                                           |
| ------------------------ | --------------------------------------------------------------------- |
| `scan`                   | Sequentially scans the entire key space                               |
| `cyclic_working_set`     | Repeatedly accesses a working set that fits in the cache              |
| `just_over_cache`        | Cycles through a working set slightly larger than the cache           |
| `hot_cold`               | Most requests target a small hot set; the rest target cold keys       |
| `zipf`                   | Generates requests with Zipf-distributed key popularity               |
| `strong_zipf`            | Stronger Zipf distribution with higher popularity concentration       |
| `random`                 | Uniformly random accesses across all keys                             |
| `burst`                  | Repeated bursts over randomly selected working sets                   |
| `switching_working_sets` | Periodically switches between several working sets                    |
| `returning_working_sets` | Cycles through several working sets and returns to previous ones      |
| `alternating_regions`    | Alternates between two distant key regions                            |
| `drifting_popularity`    | The hot region gradually moves through the key space                  |
| `flash_crowd`            | A temporary burst of popularity appears in the middle of the workload |
| `recent_history_reuse`   | Frequently reuses keys from recent request history                    |
| `delayed_history_reuse`  | Reuses keys after a fixed delay                                       |
| `periodic_hot_set`       | A hot set becomes active periodically for short intervals             |
| `three_frequency_tiers`  | Splits keys into hot, warm, and cold popularity tiers                 |
| `local_walk`             | Moves between nearby keys, creating spatial locality                  |
| `scan_with_reuse`        | Sequential scan mixed with repeated accesses to a small set           |
| `two_phase`              | Uses one working set in the first half and another in the second      |
| `hot_noise`              | Mostly accesses a hot set with random noise                           |
| `repeated_scan`          | Repeatedly scans a cache-sized working set                            |
| `mixed_scan_random`      | Combines sequential scanning with random accesses                     |
| `working_set_growth`     | Gradually increases the working-set size                              |
| `working_set_shrink`     | Gradually decreases the working-set size                              |
| `rotating_hot_sets`      | Sequentially rotates popularity between different hot regions         |
| `random_bursts`          | Random keys appear in short repeated bursts                           |
| `clustered_random`       | Random accesses are concentrated within temporary key clusters        |
| `looping_working_set`    | Repeatedly accesses one working set in a shuffled order               |

It's possible to decide how to divide cache capacity between its levels. Supported capacity sharing policies are:

| Policy      | Description                                           |
| ------------| ----------------------------------------------------- |
| `equal`     | Each level has the same size                          |
| `geometric` | Each next level is twice as large as the previous one |

### Conclusion

In our case, when each level has the same access complexity, there is absolutely no need to use more than 2 cache levels.

For two levels of caching, LFU is the best option for the first level. For the second level, however, it depends on your strategy for dividing the available space between the levels.

More advanced algorithms like ARC, LIRS, and 2Q are the best for some specific scenarios. Still, simple strategies like LRU and LFU outperform them in most tests.

The best choice depends on your data pattern. If you know which patterns are more typical for your purposes, then you can stick with the best configuration on the specific tests. With more complex environments, it's worth using ARC or LIRS since they are made to be adaptive for various data patterns.

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://dl.acm.org/doi/10.5555/1090694.1090708)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](https://dl.acm.org/doi/10.5555/645920.672996)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.

- Song Jiang and Xiaodong (Frank) Zhang. "[LIRS: An Efficient Low Inter-Reference Recency Set Replacement Policy to Improve Buffer Cache Performance](https://dl.acm.org/doi/10.1145/511399.511340)." In *ACM SIGMETRICS Performance Evaluation Review*, 2002, vol. 30, pp. 31–42.

- Song Jiang and Xiaodong Zhang. "[Making LRU Friendly to Weak Locality Workloads: A Novel Replacement Algorithm to Improve Buffer Cache Performance](https://www.computer.org/csdl/journal/tc/2005/08/t0939/13rRUy3xY7k)." In *IEEE Transactions on Computers*, 2005, vol. 54, no. 8, pp. 939-952.

- Arjun Singh Saud. "[Survey Inter-Reference Recency Based Page Replacement Policies to Cope with Weak Locality Workloads](https://www.nepjol.info/index.php/kjem/article/view/22017)." In *Kathford Journal of Engineering and Management*, 2018, vol. 1, no. 1, pp. 23-26.

- L. A. Belady and F. P. Palermo. "[On-Line Measurement of Paging Behavior by the Multivalued MIN Algorithm](https://ieeexplore.ieee.org/document/5391336)." In *IBM Journal of Research and Development*, 1974, vol. 18, no. 1, pp. 2–19.
