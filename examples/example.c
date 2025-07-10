#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splitcache.h"

int main() {
    const char* db_path = "/tmp/splitcache_example_db";

    // Open the database
    SplitCache* cache = splitcache_open(db_path);
    if (cache == NULL) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    // Put a key-value pair
    const char* key = "hello";
    const char* value = "world";
    if (splitcache_put(cache, key, value) != 0) {
        fprintf(stderr, "Failed to put value.\n");
        splitcache_close(cache);
        return 1;
    }
    printf("Stored value: '%s' for key: '%s'\n", value, key);

    // Get the value
    char* retrieved_value = splitcache_get(cache, key);
    if (retrieved_value != NULL) {
        printf("Retrieved value: '%s'\n", retrieved_value);
        free(retrieved_value); // Important: free the memory allocated by splitcache_get
    } else {
        printf("Key not found.\n");
    }

    // Close the database
    splitcache_close(cache);

    // Clean up the database files for the example
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", db_path);
    system(command);

    return 0;
}