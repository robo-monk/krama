#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// external void printf(void);

void println_ptr_char(char* str)
{
  printf("%s\n",str);
};

void println_ptr_char_ptr_char(char* fmt, char* str)
{
  printf(fmt,str);
  printf("\n");
};

long bing_long(long hello)
{
  return hello;
};

char bing_char(char hello)
{
  return hello;
};

void* malloc_long(long size)
{
  return malloc(size);
};

int id_ptr_void(void* player)
{
  return *((int*)(player));
};

void set_id_ptr_void_long(void* player, long id)
{
  *((int*)(player)) = id;
};

char* name_ptr_void(void* player)
{
  return (char*)(player + 4);
};

char* new_string_long(long len)
{
  return (char*)(malloc(len));
};

void set_char_ptr_char_char_long(char* str, char c, long idx)
{
  *((char*)(str + idx)) = c;
};

int main(void)
{
  char* s = new_string_long(16);
  set_char_ptr_char_char_long(s,'h',0);
  set_char_ptr_char_char_long(s,'e',1);
  set_char_ptr_char_char_long(s,'l',2);
  set_char_ptr_char_char_long(s,'l',3);
  set_char_ptr_char_char_long(s,'o',4);
  println_ptr_char_ptr_char((char*)("%s"),s);
  return 0;
};
