# SplitCache

SplitCache is a simple in-memory key-value store, written in C. It is designed to be integrated into other language runtimes and SDKs as a shared library.

## Structure

- `src/`: Source files
- `include/`: Header files
- `lib/`: Shared library output
- `tests/`: Test files
- `vendor/`: Third-party libraries
- `Makefile`: Build script

## Prerequisites

- `gcc`
- `make`
- `libleveldb-dev` (Debian/Ubuntu)

## Building

To build the shared library, run:

```bash
make all
```

This will create `lib/libsplitcache.so`.

## Testing

To build and run the tests, run:

```bash
make test
```

## Cleaning

To clean the build artifacts, run:

```bash
make clean
```