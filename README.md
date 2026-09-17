# NHWC

A cache algorithms research homework from [Kostantin Vladimirov's course](https://github.com/tilir/cpp-graduate). Co-authored with [Sergey Kovalenko](https://github.com/serhiosmol) and [Dmitry Lominin](https://github.com/LomininD).


## Building

You need to have [CMake](https://cmake.org) 3.14+, compatible build tools (see [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for more information) and a compiler with C++20 support installed.

```shell
cmake -B build
cmake --build build
```

## Testing

We use [GoogleTest](https://google.github.io/googletest) framework for our tests. You can run them with:
```shell
ctest --test-dir build --output-on-failure
```

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://dl.acm.org/doi/10.5555/1090694.1090708)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](https://dl.acm.org/doi/10.5555/645920.672996)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.

- Song Jiang and Xiaodong (Frank) Zhang. "[LIRS: An Efficient Low Inter-Reference Recency Set Replacement Policy to Improve Buffer Cache Performance](https://dl.acm.org/doi/10.1145/511399.511340)." *ACM SIGMETRICS Performance Evaluation Review*, vol. 30, pp. 31–42, 2002.
