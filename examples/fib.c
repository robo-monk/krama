#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// external void printf(void);

void* new_player_long_char_(long id, char* name)
{
  long size = 8 + 8;
  void* p = ((void*) malloc(size));
  *(((long*) p)) = id;
  *(((char**) p + 8)) = name;
  return p;
};

long id_void_(void* player)
{
  return *((long*) player);
};

char* name_void_(void* player)
{
  return *((char**) player + 8);
};

int main(void)
{
  char* b = "hello there people";
  void* p = new_player_long_char_(15,"mike");
  printf("player name is: %s",name_void_(p));
  return ((int) 0);
};
