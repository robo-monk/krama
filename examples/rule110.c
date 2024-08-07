#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


long* assign_long__long(long* a, long b)
{
  long* previous_a = a;
  *a = b;
  return previous_a;
};

int main(void)
{
  long* test = ((long*) malloc(8));
  assign_long__long(test,5);
  return ((int) 0);
};
