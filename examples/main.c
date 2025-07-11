#include <stdio.h>
#include <stdlib.h>
#include "levelcache.h"

#include "log.h"

int main() {
    const char* db_path = "/tmp/my_project_db";

    // Open the database with a 10MB cache
    LevelCache* cache = levelcache_open(db_path, 10, 0, 0, LOG_INFO);
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
