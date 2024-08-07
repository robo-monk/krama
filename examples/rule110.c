#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// external void printf(void);

long add_long_long(long a, long b)
{
  return ((long) a + b);
};

long sub_long_long(long a, long b)
{
  return ((long) a - b);
};

long mul_long_long(long a, long b)
{
  return ((long) a * b);
};

long div_long_long(long a, long b)
{
  return ((long) a / b);
};

bool gt_long_long(long a, long b)
{
  return ((bool) a > b);
};

bool gte_long_long(long a, long b)
{
  return ((bool) a >= b);
};

bool lt_long_long(long a, long b)
{
  return ((bool) a < b);
};

bool lte_long_long(long a, long b)
{
  return ((bool) a <= b);
};

char* add_char__char_(char* a, char* b)
{
  long len_a = ((long) strlen(a));
  long len_b = ((long) strlen(b));
  long total = add_long_long(len_a,add_long_long(len_b,1));
  char* result = ((char*) malloc(total));
  strcpy(result, a);
  strcat(result, b);
  return result;
};

int main(void)
{
  long a = add_long_long(1,mul_long_long(2,3));
  bool b = gt_long_long(a,1);
  char* bob = "bob";
  char* dylan = "dylan";
  printf("=> %ld\n",a);
  printf("name is: %s",add_char__char_(bob,dylan));
  return ((int) 0);
};
