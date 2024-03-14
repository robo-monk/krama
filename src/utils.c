#include "utils.h"
#include "stdio.h"
#include "stdlib.h"

bool is_digit(char c) { return c >= '0' && c <= '9'; }

int parse_digit(char c) {
  if (is_digit(c)) {
    return c - '0';
  }
  printf("%c is not a number", c);
  exit(1);
}

int str2int(string str, int len) {
  int n = 0;
  for (int i = 0; i < len; i++) {
    n = n * 10 + parse_digit(str[i]);
  }
  return n;
}

Vec new_vec(size_t capacity, size_t el_size) {
  return (Vec){.capacity = capacity,
               .ptr = calloc(capacity, el_size),
               .size = 0,
               .el_size = el_size};
}

void vector_push(Vec *vec, void *el) {
  if (vec->size + 1 > vec->capacity) {
    vec->capacity *= 2;
    vec->ptr = realloc(vec->ptr, vec->capacity * vec->el_size);
  }
  vec->ptr[vec->size++] = el;
}

void *vector_at(Vec *vec, int idx) {
  if (idx >= vec->size) {
    printf("\nidx bigger that vec size.\n");
    exit(1);
  }
  return vec->ptr[idx];
}
