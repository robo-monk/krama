#include "LiteralType.h"
#include "stdio.h"
#include "string.h"

LiteralType str_to_literal_type(string str) {
  for (const TypeMap *map = type_map; map->str != NULL; ++map) {
    if (strcmp(map->str, str) == 0) {
      return map->type;
    }
  }
  printf("\n Unrecognised type: %s \n", str);
  return NULL;
}

const string literal_type_to_str(LiteralType type) {
  for (const TypeMap *map = type_map; map->str != NULL; ++map) {
    if (type == map->type) {
      return map->str;
    }
  }
  return NULL;
}

string literal_val2str(LiteralStatement literal) {
  string str = malloc(20 * sizeof(char));
  switch (literal.type) {
  case LiteralType_i32:
    sprintf(str, "%d", literal.i32_value);
    break;
  }
  return str;
}
