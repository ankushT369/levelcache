# LevelCache

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

LevelCache is a high-performance, ephemeral, embedded key-value caching library written in C. It is built on top of Google's [LevelDB](https://github.com/google/leveldb) and provides a simple, clean API for caching with Time-to-Live (TTL) support.

## Features

- **Time-to-Live (TTL)**: Set an expiration time for each key, after which it is automatically considered invalid and deleted upon access.
- **Memory Management**: Control the maximum memory usage of the LevelDB cache to manage your application's footprint.
- **Simple C API**: A straightforward and easy-to-use function set for `open`, `close`, `put`, `get`, and `delete` operations.
- **High Performance**: Optimized for fast read and write operations. See the [Performance](#performance) section for details.
- **Well-Tested**: Includes a comprehensive test suite using the Google Test framework.

## Getting Started

Getting started with levelcache is simple and a step by step guide is documented in [examples](examples) directory.

## Makefile Targets

- `all`: Builds the `liblevelcache.a` static library.
- `test`: Builds and runs the Google Test suite.
- `benchmark`: Builds and runs the performance benchmark suite.
- `clean`: Removes all build artifacts.

## Performance

The following benchmarks were run on a 16-core machine with a 100MB database cache. The results are the mean of 3 repetitions.

To run the benchmarks on your own machine, use `make benchmark`.

| Operation | Throughput (ops/sec) | p50 Latency | p90 Latency | p95 Latency | p99 Latency |
|-----------|------------------------|-------------|-------------|-------------|-------------|
| **Write** | ~224,000               | ~3.6 µs     | ~4.6 µs     | ~5.0 µs     | ~6.4 µs     |
| **Read**  | ~2,016,000             | ~0.3 µs     | ~1.1 µs     | ~2.0 µs     | ~2.9 µs     |

## Contributing

Contributions are welcome! Feel free to open an issue to report a bug or suggest a feature, or submit a pull request with your improvements.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.