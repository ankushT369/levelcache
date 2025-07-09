# SplitCache Example

This example demonstrates how to use the SplitCache library.

## Compile and Run

First, build the SplitCache shared library from the root of the project:
```bash
make all
```

Then, compile the example program and link it against the library:
```bash
gcc -I../include -o example example.c -L../lib -lsplitcache
```

Finally, run the example. You will need to set the `LD_LIBRARY_PATH` so the linker can find the shared library:
```bash
export LD_LIBRARY_PATH=$PWD/../lib
./example
```
