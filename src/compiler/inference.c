#include "inference.h"
#include "../frontend/LiteralType.h"
#include "../frontend/parser.h"
#include "../hashmap/hashmap.h"
#include "CCompiler.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdlib.h>

void report_inference_error(Token token, string error_msg) {
  string error_line = line_of_token(token);
  throw_hard_error("%s\n[%d:%d] Inference error: %s\n", error_line,
                   token.row_idx, token.col_idx, error_msg);
}

Inferer Inferer_new(Compiler *com) {
  Inferer inf = {.current_stmt = NULL, .com = com};
  return inf;
}

void Inferer_throw(Inferer *inferer, const char *fmt, ...) {
  char buffer[1024];   // Buffer to hold the formatted error message
  va_list args;        // To handle the variable argument list
  va_start(args, fmt); // Initialize the variable arguments list
  vsnprintf(buffer, sizeof(buffer), fmt, args); // Create formatted string
  va_end(args); // Clean up the variable arguments list

  report_inference_error(inferer->current_stmt->token, buffer);
}

LiteralType infer_block_statement(Inferer *inf, BlockStatement *block) {
  LiteralType previous_return_type = -1;
  for (int i = 0; i < block->len; i++) {
    // TODO
    // if statement has potential to return
    // infer its type
    // if its different throw error that block statement has ambigious return
    // type

    // for now only infer last stmt in block
    if (i == block->idx) {
      previous_return_type = infer_statement(inf, block->statements[i]);
    }
  }
  return previous_return_type;
}

LiteralType infer_bin_op(Inferer *inf, LiteralType type_a, LiteralType type_b,
                         OpType op) {
  if (type_a != type_b) {
    Inferer_throw(inf, "ambgious binary operation");
  }
  return type_a;
}

LiteralType infer_conditional(Inferer *inf, ConditionalStatement *conditional) {
  LiteralType if_type = infer_block_statement(inf, conditional->if_body);

  LiteralType else_type = infer_block_statement(inf, conditional->else_body);
  if (if_type != else_type) {
    Inferer_throw(inf,
                  "ambgious conditional. If statement returns (%s) else stmt "
                  "returns (%s)",
                  literal_type_to_str(if_type), literal_type_to_str(else_type));
  }

  return if_type;
}

LiteralType infer_def_invoke(Inferer *inf, Statement *stmt) {
  DefSymbol *defsym = Compiler_defsym_get(inf->com, stmt->sym_decl.name);
  if (defsym == NULL) {
    Inferer_throw(inf, "did not find symbol %s", stmt->sym_decl.name);
  }
  return defsym->type;
}

LiteralType infer_var_read(Inferer *inf, Statement *stmt) {
  VariableSymbol *varsym = Compiler_varsym_get(inf->com, stmt->sym_decl.name);
  if (varsym == NULL) {
    Inferer_throw(inf, "did not find varsymbol %s", stmt->sym_decl.name);
  }
  return varsym->type;
}

LiteralType infer_statement(Inferer *inf, Statement *stmt) {
  inf->current_stmt = stmt;
  switch (stmt->type) {
  case STMT_BLOCK:
    return infer_block_statement(inf, stmt->block);
    break;
  //   return compile_block_statement(com, stmt->block);
  case STMT_LITERAL:
    return stmt->literal.type;
  case STMT_BINARY_OP:
    return infer_bin_op(inf, infer_statement(inf, stmt->left),
                        infer_statement(inf, stmt->right),
                        stmt->token.value.op_type);
    // case STMT_VARIABLE_DECL:
    //   return com_declare_variable(com, stmt->sym_decl.name,
    //   stmt->sym_decl.type,
    //                               com_statement(com, stmt->right));

  case STMT_VARIABLE_READ:
    return infer_var_read(inf, stmt);
    // case STMT_VARIABLE_WRITE:
    //   return compile_write_variable(com, stmt->sym_decl.name,
    //   stmt->sym_decl.type,
    //                                 com_statement(com, stmt->right));

  case STMT_DEF_INVOKE:
    return infer_def_invoke(inf, stmt);
  case STMT_CONDITIONAL:
    // printf("\neval conditional:\n");
    // dbg_stmt(stmt);
    return infer_conditional(inf, stmt->conditional);
    // case STMT_DEF_DECL:
    //   return com_def_declaration(com, stmt->sym_decl.name, stmt->right);
  }

  // report_syntax_error(stmt->token, "unsupported token");
  // Compiler_throw(com, "unsupportd token");
  Inferer_throw(inf, "could not infer type for expression");
}
