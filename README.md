# SplitCache

SplitCache is a key-value store C library built on top of Google's LevelDB. It provides a simple API for storing, retrieving, and deleting data, with added support for Time-to-Live (TTL) on keys and memory usage control for the underlying database cache.

## Structure

- `src/`: Source files for the SplitCache library.
- `include/`: Public header files.
- `lib/`: Compiled static library output (`libsplitcache.a`).
- `tests/`: Google Test suite for the library.
- `vendor/`: Third-party libraries (LevelDB, Google Test).
- `examples/`: Example usage of the library.
- `Makefile`: Build script for the library and tests.

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
- `clean`: Removes all build artifacts.

## Usage

For detailed API usage, please refer to the public header file at `include/splitcache.h` and the implementation in `src/splitcache.c`.

A practical demonstration is available in `examples/example.c`.