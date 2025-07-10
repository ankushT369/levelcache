# Using levelcache in your C project

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

Here is a sample `main.c` to get you started. It demonstrates how to open a database, put a value, get it back, and close the database.

```c
// main.c
#include <stdio.h>
#include <stdlib.h>
#include "levelcache.h"

int main() {
    const char* db_path = "/tmp/my_project_db";

    // Open the database with a 10MB cache
    LevelCache* cache = levelcache_open(db_path, 10);
    if (cache == NULL) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    // Put a key-value pair
    const char* key = "greeting";
    const char* value = "Hello from levelcache!";
    if (levelcache_put(cache, key, value, 0) != 0) { // 0 for no TTL
        fprintf(stderr, "Failed to put value.\n");
        levelcache_close(cache);
        return 1;
    }
    printf("Stored: '%s' -> '%s'\n", key, value);

    // Get the value back
    char* retrieved_value = levelcache_get(cache, key);
    if (retrieved_value != NULL) {
        printf("Retrieved: '%s' -> '%s'\n", key, retrieved_value);
        free(retrieved_value);
    } else {
        printf("Key '%s' not found.\n", key);
    }

    // Close the database
    levelcache_close(cache);

    return 0;
}
```

## 4. Compile and Run

To compile your program, you need to tell the compiler where to find the `levelcache.h` header and the `liblevelcache.so` library.

The `-I` flag points to the include directory, `-L` points to the library directory, and `-llevelcache` links the library.

```bash
gcc main.c -I./vendor/levelcache/include -L./vendor/levelcache/lib -llevelcache
```

Before running, you must update the `LD_LIBRARY_PATH` environment variable so the system's dynamic linker can find the shared library at runtime.

```bash
export LD_LIBRARY_PATH=$PWD/vendor/levelcache/lib
./a.out
```

You should see the following output:
```
Stored: 'greeting' -> 'Hello from levelcache!'
Retrieved: 'greeting' -> 'Hello from levelcache!'
```
