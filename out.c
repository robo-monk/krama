#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


char add_char_long(char gen1, long gen2)
{
  gT b = gen2;
  return ((char) gen2);
}

void main(void)
{
  char bing = add_char_long('a',4);
}
