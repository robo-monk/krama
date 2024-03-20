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
  if (vec->size >= vec->capacity) {
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

void **vector_alloc_to_array(Vec *vec) {
  void **ary_ptr = calloc(vec->size, vec->el_size);

  for (int i = 0; i < vec->size; i++) {
    ary_ptr[i] = vector_at(vec, i);
  }
  return ary_ptr;
}

string str_vec_at(StrVec *vec, int idx) { return vector_at(vec, idx); }
void str_vector_push(Vec *vec, string el) { vector_push(vec, el); }

char *str_vector_join(StrVec *vec) {
  if (vec->size == 0) {
    return strdup(""); // or calloc(1, sizeof(char)) for an empty string
  }

  // Initially allocate accum with enough space for a reasonable guess at the
  // final size This might need to be adjusted based on the expected average
  // string length
  char *accum = calloc(1, sizeof(char)); // Start with an empty string
  for (int i = 0; i < vec->size; i++) {
    char *element =
        str_vec_at(vec, i); // Correctly retrieve the string using vector_at
    char *newAccum = concat(2, accum, element);
    // free(accum); // Free the old accum to prevent memory leaks
    accum = newAccum;
  }
  return accum;
}

StrVec new_str_vec(size_t capacity) {
  return new_vec(capacity, sizeof(string));
}

void Vec_free(Vec *vec) {
  // Safely handle null pointer
  if (vec == NULL)
    return;

  // Free the allocated memory block pointed to by ptr
  free(vec->ptr);

  // Reset the Vec fields to indicate it's no longer valid.
  vec->ptr = NULL;
  vec->size = 0;
  vec->capacity = 0;
  vec->el_size = 0;
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

void throw_hard_error(const char *format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, "\033[31m\n");  // Start red text
  vfprintf(stderr, format, args); // Print formatted output in red
  fprintf(stderr, "\033[0m\n");   // Reset to default text color

  va_end(args);
  exit(1);
}
