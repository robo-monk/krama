#ifndef OP_H
#define OP_H

#include "../utils.h"
// #include "../ast/AbstractSyntaxTree.h"

typedef enum {
  OpType_ADD,
  OpType_SUB,
  OpType_MUL,
  OpType_DIV,
  OpType_MOD,

  OpType_LT,
  OpType_LTE,
  OpType_GT,
  OpType_GTE,
  OpType_EQ,

  OpType_BIN_AND,
  OpType_BIN_OR,

  OpType_BW_AND,
  OpType_BW_OR,
  OpType_BW_XOR,
} OpType;

typedef struct {
  const string str;
  OpType type;
} OpMap;

static const OpMap op_map[] = {
    {"+", OpType_ADD},  //
    {"-", OpType_SUB},  //
    {"*", OpType_MUL},  //
    {"/", OpType_DIV},  //
    {"%%", OpType_MOD}, //

    {"<", OpType_LT},   //
    {"<=", OpType_LTE}, //
    {">", OpType_GT},   //
    {">=", OpType_GTE}, //

    {"&&", OpType_BIN_AND}, //
    {"||", OpType_BIN_OR},  //

    {">", OpType_GT},   //
    {">=", OpType_GTE}, //

    {"==", OpType_EQ}, //
};

#define OP_MAP_SIZE (sizeof(op_map) / sizeof(OpMap))

OpType str_to_optype(string str);
const string optype_to_str(OpType optype);

#endif
