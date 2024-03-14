#include "utils.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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

Vec *vector_merge(Vec *a, Vec *b) {
  if (a->el_size != b->el_size) {
    printf("cannot merge vectors with different el sizes!");
    exit(1);
  }

  Vec *larger_vec = b;
  Vec *other_vec = a;
  if (a->capacity > b->capacity) {
    larger_vec = a;
    other_vec = b;
  }

  // iterate over a and push every element to b
  for (int i = 0; i < other_vec->size; i++) {
    vector_push(larger_vec, vector_at(other_vec, i));
  }

  free(other_vec);
  return larger_vec;
}

string str_vec_at(StrVec *vec, int idx) { return vector_at(vec, idx); }
void str_vector_push(Vec *vec, string el) { vector_push(vec, el); }
string str_vector_join(Vec *vec) {
  string accum;
  for (int i = 0; i < vec->size; i++) {
    accum = concat(2, accum, vec[i]);
  }
  return accum;
}

StrVec new_str_vec(size_t capacity) {
  return new_vec(capacity, sizeof(string));
}

char *concat(int count, ...) {
  va_list ap;
  int i;

  // Find required length to store merged string
  int len = 1; // room for NULL
  va_start(ap, count);
  for (i = 0; i < count; i++)
    len += strlen(va_arg(ap, char *));
  va_end(ap);

  // Allocate memory to concat strings
  char *merged = calloc(sizeof(char), len);
  int null_pos = 0;

  // Actually concatenate strings
  va_start(ap, count);
  for (i = 0; i < count; i++) {
    char *s = va_arg(ap, char *);
    strcpy(merged + null_pos, s);
    null_pos += strlen(s);
  }
  va_end(ap);

  return merged;
}
