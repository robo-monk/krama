#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void* add(long a, long b)
{
  return a+b;
}

void* add(char a, char b)
{
  return a+b;
}
