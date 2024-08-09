#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void add_long_long(long add_1, long add_2)
{
  long num = 5;
}

void add_char_char(char add_1, char add_2)
{
  char ch = 'a';
}

void add_char_long(char add_1, long add_2)
{
  // not implemented;
}

void main(void)
{
  void c = add_long_long(0,5);
  void a = add_char_char('a','b');
  void bing = add_char_long('a',4);
}
