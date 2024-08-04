#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void* array_new(void)
{
  return malloc(1024);
};

void* array_get(void* array, long index, long size)
{
  return array + index * size;
};

void array_set(void* array, long index, long size, char value)
{
  void* ptr = array + index * size;
};

void main(void)
{
  void* a = array_new();
  void* contents = array_get(a,5,8);
};
