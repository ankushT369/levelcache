# Getting Started with LevelCache

This guide provides the steps to set up and use `levelcache` as a dependency in an external C project.

## 1. Create Your C Project

First, create a directory for your project. We'll call it `example`.

```bash
mkdir example
cd example
touch main.c
```

## 2. Add and Build levelcache

Create a `vendor` directory and clone the `levelcache` repository into it. The `--recurse-submodules` flag is necessary to pull in dependencies like `leveldb`.

```bash
mkdir vendor
cd vendor
git clone --recurse-submodules https://github.com/arpitbbhayani/levelcache.git
cd levelcache
make all
cd ../..
```

This will create the shared library at `vendor/levelcache/lib/liblevelcache.so`.

## 3. Write Your Application Code

Copy the content of `main.c` present in examples direcotry and save it as `main.c` in your example project.

## 4. Compile and Run

To compile your program, you need to tell the compiler where to find the `levelcache.h` header and the `liblevelcache.so` library.

The `-I` flag points to the include directory, `-L` points to the library directory, and `-llevelcache` links the library.

```bash
gcc main.c \
    -I./vendor/levelcache/include \
    -I./vendor/levelcache/vendor/leveldb/include \
    -L./vendor/levelcache/lib \
    -L./vendor/levelcache/vendor/leveldb \
    -llevelcache -lleveldb -lstdc++ -lpthread
```

Before running, you must update the `LD_LIBRARY_PATH` environment variable so the system's dynamic linker can find the shared library at runtime.

```bash
./a.out
```

You should see the following output:
```
Stored: 'greeting' -> 'Hello from levelcache!'
Retrieved: 'greeting' -> 'Hello from levelcache!'
```
