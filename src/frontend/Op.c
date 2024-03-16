#include "Op.h"
#include "stdio.h"
#include "string.h"

OpType str_to_optype(string str) {
  for (int i = 0; i < OP_MAP_SIZE; i++) {
    if (strcmp(op_map[i].str, str) == 0) {
      return op_map[i].type;
    }
  }

  // printf("unrecognised operand string '%s'", str);
  return -1;
}

const string optype_to_str(OpType optype) {
  for (int i = 0; i < OP_MAP_SIZE; i++) {
    if (optype == op_map[i].type) {
      return op_map[i].str;
    }
  }
  printf("\nfound unrecognised optype\n");
  return NULL;
}
