#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
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
    leveldb_t *db = splitcache_open(DB_PATH);
    assert(db != NULL);

    const char *key = "test_key";
    const char *value = "test_value";

    assert(splitcache_put(db, key, value, strlen(value) + 1) == 0);

    void *retrieved_value = NULL;
    size_t retrieved_len = 0;

    assert(splitcache_get(db, key, &retrieved_value, &retrieved_len) == 0);
    assert(strcmp((char*)retrieved_value, value) == 0);
    assert(retrieved_len == strlen(value) + 1);

    free(retrieved_value);
    splitcache_close(db);
    teardown();
    printf("PASSED\n");
}

void test_get_non_existent() {
    printf("Running test: test_get_non_existent\n");
    setup();
    leveldb_t *db = splitcache_open(DB_PATH);
    assert(db != NULL);

    const char *key = "non_existent_key";
    void *retrieved_value = NULL;
    size_t retrieved_len = 0;

    assert(splitcache_get(db, key, &retrieved_value, &retrieved_len) == -1);
    assert(retrieved_value == NULL);

    splitcache_close(db);
    teardown();
    printf("PASSED\n");
}

void test_delete() {
    printf("Running test: test_delete\n");
    setup();
    leveldb_t *db = splitcache_open(DB_PATH);
    assert(db != NULL);

    const char *key = "test_key";
    const char *value = "test_value";

    assert(splitcache_put(db, key, value, strlen(value) + 1) == 0);
    assert(splitcache_delete(db, key) == 0);

    void *retrieved_value = NULL;
    size_t retrieved_len = 0;

    assert(splitcache_get(db, key, &retrieved_value, &retrieved_len) == -1);
    assert(retrieved_value == NULL);

    splitcache_close(db);
    teardown();
    printf("PASSED\n");
}

int main() {
    test_put_and_get();
    test_get_non_existent();
    test_delete();
    return 0;
}