#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void* add_long_long(long add_1, long add_2)
{
  return a+b;
}

void* add_char_char(char add_1, char add_2)
{
  return a+b;
}

char hello_char(char a)
{
  return a;
}

void main(void)
{
  void* b = add_long_long(0,5);
  void* c = add_long_long(0,5);
  void* a = add_char_char('a','b');
  char should_be_char = hello_char('a');
  char should_be_i64 = hello_char(5);
}
