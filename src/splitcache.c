#include "splitcache.h"
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1024

typedef struct entry {
    char *key;
    void *value;
    size_t value_len;
    struct entry *next;
} entry_t;

struct ht_t {
    entry_t **entries;
};

unsigned int hash(const char *key) {
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    for (; i < key_len; ++i) {
        value = value * 37 + key[i];
    }

    return value % TABLE_SIZE;
}

ht_t *ht_create(void) {
    ht_t *hashtable = malloc(sizeof(ht_t));
    hashtable->entries = malloc(sizeof(entry_t*) * TABLE_SIZE);

    for (int i = 0; i < TABLE_SIZE; ++i) {
        hashtable->entries[i] = NULL;
    }

    return hashtable;
}

void ht_set(ht_t *hashtable, const char *key, const void *value, size_t value_len) {
    unsigned int slot = hash(key);
    entry_t *entry = hashtable->entries[slot];

    if (entry == NULL) {
        hashtable->entries[slot] = malloc(sizeof(entry_t));
        hashtable->entries[slot]->key = strdup(key);
        hashtable->entries[slot]->value = malloc(value_len);
        memcpy(hashtable->entries[slot]->value, value, value_len);
        hashtable->entries[slot]->value_len = value_len;
        hashtable->entries[slot]->next = NULL;
        return;
    }

    entry_t *prev;
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            free(entry->value);
            entry->value = malloc(value_len);
            memcpy(entry->value, value, value_len);
            entry->value_len = value_len;
            return;
        }
        prev = entry;
        entry = prev->next;
    }

    prev->next = malloc(sizeof(entry_t));
    prev->next->key = strdup(key);
    prev->next->value = malloc(value_len);
    memcpy(prev->next->value, value, value_len);
    prev->next->value_len = value_len;
    prev->next->next = NULL;
}

int ht_get(ht_t *hashtable, const char *key, void **value, size_t *value_len) {
    unsigned int slot = hash(key);
    entry_t *entry = hashtable->entries[slot];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            *value = malloc(entry->value_len);
            memcpy(*value, entry->value, entry->value_len);
            *value_len = entry->value_len;
            return 0;
        }
        entry = entry->next;
    }

    return -1;
}

void ht_free(ht_t *hashtable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        entry_t *entry = hashtable->entries[i];
        while (entry != NULL) {
            entry_t *next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }
    free(hashtable->entries);
    free(hashtable);
}



int ht_delete(ht_t *hashtable, const char *key) {
    unsigned int slot = hash(key);
    entry_t *entry = hashtable->entries[slot];
    entry_t *prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            if (prev == NULL) {
                hashtable->entries[slot] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry->key);
            free(entry->value);
            free(entry);
            return 0; // Found and deleted
        }
        prev = entry;
        entry = entry->next;
    }

    return -1; // Not found
}
