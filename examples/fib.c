#include <stdio.h>


// external void printf(void* n0, void* n1);

unsigned long factorial(unsigned long n)
{
  if (n < 1) {
  return 1;
};
  return factorial(n - 1) * n;
};

int main(void)
{
  int result = factorial(5);
  printf("%d\n",result);
  return 1;
};
