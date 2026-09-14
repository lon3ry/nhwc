# NHWC

A cache algorithms research homework from [Kostantin Vladimirov's course](https://github.com/tilir/cpp-graduate). Co-authored with [Sergey Kovalenko](https://github.com/serhiosmol) and [Dmitry Lominin](https://github.com/LomininD).


## Building

You need to have [CMake](https://cmake.org) 3.11+ and a compiler with C++20 support installed.

```shell
cmake -B build
cmake --build build
```

## Testing

You can run tests with:
```shell
ctest --test-dir build --output-on-failure
```

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://www.usenix.org/conference/fast-03/arc-self-tuning-low-overhead-replacement-cache)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](http://www.vldb.org/conf/1994/P439.PDF)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.
