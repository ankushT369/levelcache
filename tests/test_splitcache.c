#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "splitcache.h"

const char* DB_PATH = "/tmp/splitcache_test_db";

void setup() {
    // Clean up any previous test database
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
    system(command);
}

void teardown() {
    // Clean up the test database
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
    system(command);
}

void test_put_and_get() {
    printf("Running test: test_put_and_get\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH);
    assert(cache != NULL);

    const char *key = "test_key";
    const char *value = "test_value";

    assert(splitcache_put(cache, key, value, 0) == 0);

    char *retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value != NULL);
    assert(strcmp(retrieved_value, value) == 0);

    free(retrieved_value);
    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_get_non_existent() {
    printf("Running test: test_get_non_existent\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH);
    assert(cache != NULL);

    const char *key = "non_existent_key";
    char *retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value == NULL);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_delete() {
    printf("Running test: test_delete\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH);
    assert(cache != NULL);

    const char *key = "test_key";
    const char *value = "test_value";

    assert(splitcache_put(cache, key, value, 0) == 0);
    assert(splitcache_delete(cache, key) == 0);

    char *retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value == NULL);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_ttl() {
    printf("Running test: test_ttl\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH);
    assert(cache != NULL);

    const char *key = "ttl_key";
    const char *value = "ttl_value";

    // Test that the key expires after 1 second
    assert(splitcache_put(cache, key, value, 1) == 0);
    sleep(2);
    char *retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value == NULL);

    // Test that the key does not expire before the TTL
    assert(splitcache_put(cache, key, value, 5) == 0);
    retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value != NULL);
    assert(strcmp(retrieved_value, value) == 0);
    free(retrieved_value);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

int main() {
    test_put_and_get();
    test_get_non_existent();
    test_delete();
    test_ttl();
    return 0;
}