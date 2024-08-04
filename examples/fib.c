#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// external void printf(void* fmt, void* val);

long bing_long(long hello)
{
  return hello;
};

int main(void)
{
  char* test = malloc(1024 * 8);
  *(test + 0) = 'h';
  *(test + 1) = 'e';
  *(test + 2) = 'l';
  *(test + 3) = 'l';
  *(test + 4) = 'o';
  *(test + 5) = 'o';
  *(test + 6) = 0;
  free(test);
  bing_long(1);
  return 0;
};
