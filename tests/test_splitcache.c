#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "splitcache.h"

void test_put_and_get() {
    printf("Running test: test_put_and_get\n");
    ht_t *ht = ht_create();
    const char *key = "test_key";
    const char *value = "test_value";

    ht_set(ht, key, value, strlen(value) + 1);

    void *retrieved_value = NULL;
    size_t retrieved_len = 0;

    assert(ht_get(ht, key, &retrieved_value, &retrieved_len) == 0);
    assert(strcmp((char*)retrieved_value, value) == 0);
    assert(retrieved_len == strlen(value) + 1);

    free(retrieved_value);
    ht_free(ht);
    printf("PASSED\n");
}

void test_get_non_existent() {
    printf("Running test: test_get_non_existent\n");
    ht_t *ht = ht_create();
    const char *key = "non_existent_key";
    void *retrieved_value = NULL;
    size_t retrieved_len = 0;

    assert(ht_get(ht, key, &retrieved_value, &retrieved_len) == -1);
    assert(retrieved_value == NULL);

    ht_free(ht);
    printf("PASSED\n");
}

int main() {
    test_put_and_get();
    test_get_non_existent();
    return 0;
}
