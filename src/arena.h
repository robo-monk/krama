#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ALIGNMENT (_Alignof(max_align_t))

typedef struct {
    void* data;
    size_t capacity;
    size_t offset;
    void* last_ptr;
} Arena;

Arena arena_new(size_t capacity);
void* arena_alloc(Arena *arena, size_t size);
void arena_destroy(Arena *arena);

char* arena_strdup(Arena *arena, const char* s);

#ifdef ARENA_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>

static size_t align_size(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

Arena arena_new(size_t capacity) {
    return (Arena) {
        .data = malloc(capacity),
        .capacity = capacity,
        .offset = 0,
        .last_ptr = 0
    };
}

void* arena_alloc(Arena *arena, size_t bytes) {
    size_t size = align_size(bytes);

    if (arena->offset + size > arena->capacity) {
        printf("\n arena out of memory, implement resizing or regions...");
        exit(1);
    }

    void* ptr = arena->data + arena->offset;
    arena->offset += size;
    arena->last_ptr = ptr;

    // assert(arena->data+arena->offset == arena->last_ptr+size);
    // assert(size == ((arena->data+arena->offset) - arena->last_ptr));
    return ptr;
}

void arena_destroy(Arena *arena) {
    free(arena->data);
    arena->data = NULL;
    arena->last_ptr = 0;
    arena->capacity = 0;
    arena->offset = 0;
}


char* arena_strdup(Arena *arena, const char* s) {
    size_t len = strlen(s) + 1;
    char* new = arena_alloc(arena, len);
    return (char*) memcpy(new, s, len);
}

#endif // IMPL

#endif // ARENA_H
