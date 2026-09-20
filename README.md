# NHWC

A cache algorithms research homework from [Kostantin Vladimirov's course](https://github.com/tilir/cpp-graduate). Co-authored with [Sergey Kovalenko](https://github.com/serhiosmol) and [Dmitry Lominin](https://github.com/LomininD).

This project implements multi-level cache with various algorithms supported.

## Building

You need to have [CMake](https://cmake.org) 3.14+, compatible build tools (see [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for more information) and a compiler with C++20 support installed.

```shell
cmake -B build
cmake --build build
```

## Running

First of all, you need to create a `config.txt` file with following contents:
```
<number_of_levels> <cache_algorithms>
```
For example:
```
2 ARC LRU
```
```
1 ARC
```

Following algorithms are supported:

- ARC

- 2Q

- LRU

- LFU

- LIRS

The program accepts size of cache levels, then data set size and a sequence of requests. For example:
```shell
echo 2 ARC 2Q > config.txt
echo 2 4 6 1 2 1 2 1 2 | ./build/main
```
In this case we have a two-level cache with ARC cache (size 2) and 2Q (size 4) and a sequence with 6 requests.

## Testing

We use [GoogleTest](https://google.github.io/googletest) framework for our tests. You can run them with:
```shell
ctest --test-dir build --output-on-failure
```

## Benchmarking

This project includes various benchmarks. To run them, install [uv](https://docs.astral.sh/uv) and run the following:
```shell
cd bench
uv run main.py
```
These benchmarks also run automatically, you can check the latest results [here](https://lon3ry.github.io/nhwc).

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://dl.acm.org/doi/10.5555/1090694.1090708)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](https://dl.acm.org/doi/10.5555/645920.672996)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.

- Song Jiang and Xiaodong (Frank) Zhang. "[LIRS: An Efficient Low Inter-Reference Recency Set Replacement Policy to Improve Buffer Cache Performance](https://dl.acm.org/doi/10.1145/511399.511340)." In *ACM SIGMETRICS Performance Evaluation Review*, 2002, vol. 30, pp. 31–42.

- Song Jiang and Xiaodong Zhang. "[Making LRU Friendly to Weak Locality Workloads: A Novel Replacement Algorithm to Improve Buffer Cache Performance](https://www.computer.org/csdl/journal/tc/2005/08/t0939/13rRUy3xY7k)." In *IEEE Transactions on Computers*, 2005, vol. 54, no. 8, pp. 939-952.

- Arjun Singh Saud. "[Survey Inter-Reference Recency Based Page Replacement Policies to Cope with Weak Locality Workloads](https://www.nepjol.info/index.php/kjem/article/view/22017)." In *Kathford Journal of Engineering and Management*, 2018, vol. 1, no. 1, pp. 23-26.

- L. A. Belady and F. P. Palermo. "[On-Line Measurement of Paging Behavior by the Multivalued MIN Algorithm](https://ieeexplore.ieee.org/document/5391336)." In *IBM Journal of Research and Development*, 1974, vol. 18, no. 1, pp. 2–19.
