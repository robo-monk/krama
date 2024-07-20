#ifndef HASHMAP_H
#define HASHMAP_H

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#define HASHMAP_INITIAL_CAPACITY 4096

typedef struct hash_entry_t {
    char *key;
    void *val;
    struct hash_entry_t *next;
} hash_entry_t;

typedef struct {
    size_t capacity;
    size_t count;
    hash_entry_t* entries[HASHMAP_INITIAL_CAPACITY];
} hashmap_t;


hashmap_t hashmap_create(size_t entry_size) {
    return (hashmap_t) {
        .capacity = HASHMAP_INITIAL_CAPACITY,
        .count = 0,
        .entries = { NULL }
    };
}

void __hashmap_free_entry(hash_entry_t *e) {
   if (e == NULL) return;
   free(e->key);
   __hashmap_free_entry(e->next);
   free(e);
}
void hashmap_free(hashmap_t* map) {
    for (int i = 0; i < map->capacity; i++) {
        __hashmap_free_entry(map->entries[i]);
    }
}

unsigned int hashmap_hash_string(char *key) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

void hashmap_insert(hashmap_t *map, char* key, void* value) {
    unsigned int idx = hashmap_hash_string(key) % map->capacity;
    hash_entry_t* entry = (hash_entry_t*) malloc(sizeof(hash_entry_t));
    entry->next = map->entries[idx];
    entry->key = strdup(key);
    entry->val = value;
    map->entries[idx] = entry;
    map->count++;
}

void* hashmap_get(hashmap_t *map, char* key) {
    unsigned int idx = hashmap_hash_string(key) % map->capacity;
    hash_entry_t* entry = map->entries[idx];
    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    if (entry == NULL) return NULL;
    return entry->val;
}

#endif
