#ifndef LITERALTYPE_H
#define LITERALTYPE_H

#include "../ast/AbstractSyntaxTree.h"

typedef struct {
  const string str;
  LiteralType type;
} TypeMap;

static const TypeMap type_map[] = {
    {"i32", LiteralType_i32},
    {"f64", LiteralType_f64},   //
    {"char", LiteralType_char}, //
    {"bool", LiteralType_bool}, //
    {NULL}                      //
};

#include "LiteralType.h"
#include "stdio.h"
#include "string.h"

LiteralType str_to_literal_type(string str);
const string literal_type_to_str(LiteralType type);
string literal_val2str(LiteralStatement literal);

#endif
