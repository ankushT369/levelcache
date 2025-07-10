# SplitCache

SplitCache is a key-value store C library built on top of Google's LevelDB. It provides a simple API for storing, retrieving, and deleting data, with added support for Time-to-Live (TTL) on keys and memory usage control for the underlying database cache.

## Prerequisites

- `gcc`
- `g++`
- `make`
- `cmake` (for building LevelDB)

## Building

First, fetch the required dependencies. This project uses git submodules for its dependencies.

```bash
git submodule update --init --recursive
```

To build the static library (`libsplitcache.a`), run:

```bash
make all
```

This will compile the source files and create the library in the `lib/` directory.

## Makefile Targets

The `Makefile` provides the following targets:

- `all`: Builds the `libsplitcache.a` static library.
- `test`: Builds and runs the Google Test suite.
- `benchmark`: Builds and runs the performance benchmark suite.
- `clean`: Removes all build artifacts.

## Usage

For detailed API usage, please refer to the public header file at `include/splitcache.h` and the implementation in `src/splitcache.c`.

A practical demonstration is available in `examples/example.c`.

## Performance

### Running the Benchmark

To run the benchmarks on your own machine, use the following command:

```bash
make benchmark
```

### Example Results

The following numbers were captured on a 16-core machine with a 100MB database cache. The results are the mean of 3 repetitions.

| Operation | Throughput (ops/sec) | p50 Latency | p90 Latency | p95 Latency | p99 Latency |
|-----------|------------------------|-------------|-------------|-------------|-------------|
| **Write** | ~262,000               | ~3.2 µs     | ~4.0 µs     | ~4.4 µs     | ~5.6 µs     |
| **Read**  | ~593,000               | ~1.7 µs     | ~2.3 µs     | ~2.5 µs     | ~2.9 µs     |
