#include <stdio.h>
#include <stdbool.h>


int to_i32(void* n)
{
  return ((int) n);
};

long fac(long a)
{
  if (a <= 1) {
  return 1;
};
  return a * fac(a - 1);
};

int main(void)
{
  long a = fac(5);
  printf("result~%ld",a);
  return to_i32(0,5);
};
