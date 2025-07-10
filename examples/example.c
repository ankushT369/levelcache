#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "levelcache.h"

int main() {
    const char* db_path = "/tmp/levelcache_example_db";

    // Open the database with a 10MB cache
    LevelCache* cache = levelcache_open(db_path, 10);
    if (cache == NULL) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    // Put a key-value pair with no TTL
    const char* key1 = "hello";
    const char* value1 = "world";
    if (levelcache_put(cache, key1, value1, 0) != 0) {
        fprintf(stderr, "Failed to put value.\n");
        levelcache_close(cache);
        return 1;
    }
    printf("Stored value: '%s' for key: '%s' (no TTL)\n", value1, key1);

    // Put a key-value pair with a 2-second TTL
    const char* key2 = "temporary";
    const char* value2 = "i will disappear";
    if (levelcache_put(cache, key2, value2, 2) != 0) {
        fprintf(stderr, "Failed to put value.\n");
        levelcache_close(cache);
        return 1;
    }
    printf("Stored value: '%s' for key: '%s' (2s TTL)\n", value2, key2);


    // Get the first value
    char* retrieved_value = levelcache_get(cache, key1);
    if (retrieved_value != NULL) {
        printf("Retrieved value for key '%s': '%s'\n", key1, retrieved_value);
        free(retrieved_value);
    } else {
        printf("Key '%s' not found.\n", key1);
    }

    // Get the second value
    retrieved_value = levelcache_get(cache, key2);
    if (retrieved_value != NULL) {
        printf("Retrieved value for key '%s': '%s'\n", key2, retrieved_value);
        free(retrieved_value);
    } else {
        printf("Key '%s' not found.\n", key2);
    }
    
    printf("\nWaiting for 3 seconds for TTL to expire...\n\n");
    sleep(3);

    // Try to get the second value again
    retrieved_value = levelcache_get(cache, key2);
    if (retrieved_value != NULL) {
        printf("Retrieved value for key '%s': '%s'\n", key2, retrieved_value);
        free(retrieved_value);
    } else {
        printf("Key '%s' not found (as expected, TTL expired).\n", key2);
    }


    // Close the database
    levelcache_close(cache);

    return 0;
}
