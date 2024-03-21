#include "LiteralType.h"
#include "stdio.h"
#include "string.h"

LiteralType str_to_literal_type(string str) {
  for (int i = 0; i < TYPE_MAP_SIZE; i++) {
    if (strcmp(type_map[i].str, str) == 0) {
      return type_map[i].type;
    }
  }
  return -1;
}

const string literal_type_to_str(LiteralType type) {
  for (int i = 0; i < TYPE_MAP_SIZE; i++) {
    if (type == type_map[i].type) {
      return type_map[i].str;
    }
  }

  // throw_hard_error("Unrecognised Literal Type");
  return NULL;
}

string literal_val2str(LiteralStatement literal) {
  string str = malloc(20 * sizeof(char));
  switch (literal.type) {
  case LiteralType_i32:
    sprintf(str, "%d", literal.i32_value);
    break;
  case LiteralType_f64:
    sprintf(str, "%f", literal.f64_value);
    break;
  case LiteralType_UNKNOWN:
    sprintf(str, "uknown!", literal.f64_value);
  case LiteralType_void:
    sprintf(str, "VOID");
    break;
  }
  return str;
}
