#ifndef UTILS_H
#define UTILS_H
#define bool int
#define true 1
#define false 0
#define string char *
#include "stdlib.h"

typedef struct {
  void **ptr;
  int size;
  size_t capacity;
  size_t el_size;
} Vec;

bool is_digit(char c);

int str2int(string str, int len);
int parse_digit(char c);

Vec new_vec(size_t capacity, size_t el_size);
void vector_push(Vec *vec, void *el);
void *vector_at(Vec *vec, int idx);
#endif
