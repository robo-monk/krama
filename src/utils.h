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

typedef Vec StrVec;
string str_vec_at(StrVec *vec, int idx);
void str_vector_push(Vec *vec, string el);
string str_vector_join(Vec *vec);

bool is_digit(char c);

int str2int(string str, int len);
int parse_digit(char c);

Vec new_vec(size_t capacity, size_t el_size);
StrVec new_str_vec(size_t capacity);

void vector_push(Vec *vec, void *el);
void *vector_at(Vec *vec, int idx);
char *concat(int count, ...);
#endif
