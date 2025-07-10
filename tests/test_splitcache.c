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
    SplitCache *cache = splitcache_open(DB_PATH, 0);
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
    SplitCache *cache = splitcache_open(DB_PATH, 0);
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
    SplitCache *cache = splitcache_open(DB_PATH, 0);
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
    SplitCache *cache = splitcache_open(DB_PATH, 0);
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

void test_overwrite_key() {
    printf("Running test: test_overwrite_key\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH, 0);
    assert(cache != NULL);

    const char *key = "overwrite_key";
    const char *value1 = "value1";
    const char *value2 = "value2";

    assert(splitcache_put(cache, key, value1, 0) == 0);
    char *retrieved_value = splitcache_get(cache, key);
    assert(strcmp(retrieved_value, value1) == 0);
    free(retrieved_value);

    assert(splitcache_put(cache, key, value2, 0) == 0);
    retrieved_value = splitcache_get(cache, key);
    assert(strcmp(retrieved_value, value2) == 0);
    free(retrieved_value);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_update_ttl() {
    printf("Running test: test_update_ttl\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH, 0);
    assert(cache != NULL);

    const char *key = "update_ttl_key";
    const char *value = "update_ttl_value";

    // Put with a TTL, then update with no TTL
    assert(splitcache_put(cache, key, value, 1) == 0);
    assert(splitcache_put(cache, key, value, 0) == 0);
    sleep(2);
    char *retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value != NULL);
    free(retrieved_value);

    // Put with no TTL, then update with a TTL
    assert(splitcache_put(cache, key, value, 0) == 0);
    assert(splitcache_put(cache, key, value, 1) == 0);
    sleep(2);
    retrieved_value = splitcache_get(cache, key);
    assert(retrieved_value == NULL);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_delete_non_existent() {
    printf("Running test: test_delete_non_existent\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH, 0);
    assert(cache != NULL);

    assert(splitcache_delete(cache, "non_existent_key") == 0);

    splitcache_close(cache);
    teardown();
    printf("PASSED\n");
}

void test_empty_value() {
    printf("Running test: test_empty_value\n");
    setup();
    SplitCache *cache = splitcache_open(DB_PATH, 0);
    assert(cache != NULL);

    const char *key = "empty_value_key";
    const char *value = "";

    assert(splitcache_put(cache, key, value, 0) == 0);
    char *retrieved_value = splitcache_get(cache, key);
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
    test_overwrite_key();
    test_update_ttl();
    test_delete_non_existent();
    test_empty_value();
    return 0;
}
