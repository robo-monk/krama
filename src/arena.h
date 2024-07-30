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

typedef struct {
    void* data;
    size_t element_size;
    size_t count;
    size_t capacity;
} vector_t;

vector_t vector_new_arena(Arena *arena, size_t initial_cap, size_t element_size);
vector_t vector_new(size_t initial_cap, size_t element_size);
void* vector_get(vector_t *v, size_t i);
void vector_push(vector_t *v, const void* e);
// void vector_push_ptr(vector_t *v, const size_t ptr);

void vector_push_ptr(vector_t *v, const void* ptr);
void vector_free(vector_t *v);
void* vector_to_array(vector_t *v);

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


vector_t vector_new(size_t initial_cap, size_t element_size) {
    return (vector_t) {
        .data = malloc(initial_cap * element_size),
        .element_size = element_size,
        .capacity = initial_cap,
        .count = 0
    };
}

void* vector_to_array(vector_t *v) {
    void* array = calloc(v->count, v->element_size);
    memcpy(array, v->data, v->count*v->element_size);
    return array;
}

void* vector_get(vector_t *v, size_t i) {
    void* p = v->data + v->element_size*i;
    assert(i >= 0);
    assert(p < (v->data + (v->element_size*v->count)));
    return p;
}

void vector_push(vector_t *v, const void* e) {
    if (v->capacity == v->count) {
        v->capacity *= 2;
        v->data = realloc(v->data, v->capacity);
    }
    void* p = (char*) v->data + v->element_size*(v->count++);
    memcpy(p, e, v->element_size);
}

void vector_push_ptr(vector_t *v, const void* ptr) {
    assert(v->element_size == sizeof(void*));

    if (v->capacity == v->count) {
        v->capacity *= 2;
        v->data = realloc(v->data, v->capacity);
    }

    (((size_t*) v->data))[v->count++] = (size_t) ptr;
}

void vector_free(vector_t *v) {
    free(v->data);
}


#endif // IMPL

#endif // ARENA_H
