#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splitcache.h"

int main() {
    const char* db_path = "/tmp/splitcache_example_db";

    // Open the database
    leveldb_t* db = splitcache_open(db_path);
    if (db == NULL) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    // Put a key-value pair
    const char* key = "hello";
    const char* value = "world";
    if (splitcache_put(db, key, value, strlen(value)) != 0) {
        fprintf(stderr, "Failed to put value.\n");
        splitcache_close(db);
        return 1;
    }
    printf("Stored value: '%s' for key: '%s'\n", value, key);

    // Get the value
    void* read_value = NULL;
    size_t read_len;
    if (splitcache_get(db, key, &read_value, &read_len) == 0) {
        printf("Retrieved value: '%.*s'\n", (int)read_len, (char*)read_value);
        free(read_value); // Important: free the memory allocated by splitcache_get
    } else {
        printf("Key not found.\n");
    }

    // Close the database
    splitcache_close(db);

    // Clean up the database files for the example
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", db_path);
    system(command);

    return 0;
}
