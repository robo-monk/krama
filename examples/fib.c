#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// external void printf(void* v, void* b);

void print(void* fmt, long value)
{
  printf(fmt,value);
};

void println(void* fmt, long value)
{
  printf(fmt,value);
  printf("\n","");
};

void times(long n)
{
  if (n > 1) {
  times(n - 1);
};
  println("times$%d",n);
};

long main(void)
{
  long hello = 5;
  println("helloisbefore__%d",hello);
  hello = hello + hello * 2 * hello = hello;
  println("helloisnow__%d",hello);
  return 0;
};
