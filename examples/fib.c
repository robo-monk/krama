#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int id_ptr_void(void* player)
{
  return *((int*) player);
};

int main(void)
{
  return 0;
};
