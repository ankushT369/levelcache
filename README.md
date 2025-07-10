# LevelCache: An Embedded Caching Library

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

LevelCache is a high-performance, embedded key-value caching library written in C. It is built on top of Google's [LevelDB](https://github.com/google/leveldb) storage engine and provides a simple, clean API for caching with Time-to-Live (TTL) support.

## Features

- **Time-to-Live (TTL)**: Set an expiration time for each key, after which it is automatically considered invalid and deleted upon access.
- **Memory Management**: Control the maximum memory usage of the LevelDB cache to manage your application's footprint.
- **Simple C API**: A straightforward and easy-to-use function set for `open`, `close`, `put`, `get`, and `delete` operations.
- **High Performance**: Optimized for fast read and write operations. See the [Performance](#performance) section for details.
- **Well-Tested**: Includes a comprehensive test suite using the Google Test framework.

## Getting Started

### 1. Clone the Repository

This project uses git submodules for its dependencies. Clone the repository and initialize the submodules:

```bash
git clone https://github.com/arpitbbhayani/levelcache.git
cd levelcache
git submodule update --init --recursive
```

### 2. Build the Library

To build the static library (`liblevelcache.a`), run:

```bash
make all
```

This will compile the source files and create the library in the `lib/` directory.

## Usage Example

Here is a simple example of how to use LevelCache in your C application:

```c
#include <stdio.h>
#include <stdlib.h>
#include "levelcache.h"

int main() {
    // Open the database with a 100MB cache
    LevelCache* cache = levelcache_open("/tmp/my_cache_db", 100);
    if (cache == NULL) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    // Store a value with a 60-second TTL
    levelcache_put(cache, "my_key", "my_value", 60);
    printf("Stored 'my_value' for key 'my_key'.\n");

    // Retrieve the value
    char* retrieved_value = levelcache_get(cache, "my_key");
    if (retrieved_value != NULL) {
        printf("Retrieved value: '%s'\n", retrieved_value);
        free(retrieved_value); // Caller is responsible for freeing memory
    } else {
        printf("Key not found or expired.\n");
    }

    // Close the database
    levelcache_close(cache);

    return 0;
}
```
For more detailed API documentation, please refer to the public header file at `include/levelcache.h`.

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
| **Write** | ~262,000               | ~3.2 µs     | ~4.0 µs     | ~4.4 µs     | ~5.6 µs     |
| **Read**  | ~593,000               | ~1.7 µs     | ~2.3 µs     | ~2.5 µs     | ~2.9 µs     |

## Contributing

Contributions are welcome! Feel free to open an issue to report a bug or suggest a feature, or submit a pull request with your improvements.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.