#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


long add_long_long(long gen1, long gen2)
{
  return ((long) gen1 + gen2);
}

char* add_char__char_(char* a, char* b)
{
  long len_a = ((long) strlen(a));
  long len_b = ((long) strlen(b));
  char* result = ((char*) malloc(add_long_long(len_a,add_long_long(len_b,1))));
  strcpy(result, a);
  strcat(result, b);
  return result;
}

void println_char__char_(char* fmt, char* s)
{
  printf(fmt, s);
  printf("\n");
}

void println_char_(char* s)
{
  println_char__char_("%s",s);
}

int main(void)
{
  long bing = add_long_long(5,4);
  char* hello_world = add_char__char_("hello",add_char__char_(" ","world"));
  println_char_(hello_world);
  return ((int) 0);
}
