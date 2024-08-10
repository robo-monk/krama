#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


char* malloc_long(long size)
{
  return ((char*) 0);
}

long add_long_long(long gen1, long gen2)
{
  return ((long) gen1 + gen2);
}

char* add_char__char_(char* a, char* b)
{
  long len_a = ((long) strlen(a));
  long len_b = ((long) strlen(b));
  char* result = ((char*) malloc_long(50));
  strcpy(result, a);
  strcat(result, b);
  return result;
}

int main(void)
{
  char* bing = malloc_long(5);
  long bong = add_long_long(5,4);
  char* hello_world = add_char__char_("hello",add_char__char_(" ","world"));
  return ((int) 0);
}
