#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HASHMAP_INITIAL_CAPACITY 4096

typedef struct hash_entry_t {
    char *key;
    void *val;
    struct hash_entry_t *next;
} hash_entry_t;

typedef void (*hashmap_entry_free_func)(void*);

typedef struct {
    size_t capacity;
    size_t count;
    hash_entry_t** entries;
    hashmap_entry_free_func free_func;
} hashmap_t;

// Function declarations
hashmap_t* hashmap_create(size_t entry_size, hashmap_entry_free_func free_func);
void hashmap_free_entry(hashmap_t* map, hash_entry_t *e);
void hashmap_free(hashmap_t* map);
unsigned int hashmap_hash_string(char *key);
void hashmap_insert(hashmap_t *map, char* key, void* value);
void* hashmap_get(hashmap_t *map, char* key);

// Implementation macro
#ifdef HASHMAP_IMPLEMENTATION

hashmap_t* hashmap_create(size_t entry_size, hashmap_entry_free_func free_func) {
    hashmap_t *map = (hashmap_t*) malloc(sizeof(hashmap_t));
    if (!map) {
        printf("\nMemory allocation failed wtf?\n");
        exit(1);
    }
    map->capacity = HASHMAP_INITIAL_CAPACITY;
    map->count = 0;
    map->entries = (hash_entry_t**) calloc(map->capacity, sizeof(hash_entry_t*));
    map->free_func = free_func;
    return map;
}

void hashmap_free_entry(hashmap_t* map, hash_entry_t *e) {
   while (e != NULL) {
        hash_entry_t* next = e->next;
        free(e->key);
        if (map->free_func) {
            map->free_func(e->val);
        }
        free(e);
        e = next;
   }
}

void hashmap_free(hashmap_t* map) {
    for (size_t i = 0; i < map->capacity; i++) {
        hashmap_free_entry(map, map->entries[i]);
    }
    free(map->entries);
    free(map);
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

#endif // HASHMAP_IMPLEMENTATION

#endif // HASHMAP_H
