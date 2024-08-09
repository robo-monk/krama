#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void* add_char_char(char add_1, char add_2)
{
  return assert(0);
}

long hello_long(long a)
{
  return a;
}

void main(void)
{
  void* b = add_char_char(0,5);
  void* c = add_char_char(0,5);
  void* a = add_char_char('a','b');
  long should_be_char = hello_long('a');
  long should_be_i64 = hello_long(5);
}
