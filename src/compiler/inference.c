#include "inference.h"
#include "../frontend/parser.h"
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

LiteralType infer_block_statement_with_initial(Inferer *inf,
                                               BlockStatement *block,
                                               LiteralType base) {

  if (block == NULL || block->len == 0) {
    return LiteralType_void;
  }

  printf("BLOCK Len is %d\n", block->len);
  dbg_stmt(block->statements[block->len - 1]);
  printf("\n--\n");
  LiteralType branch_return_type =
      infer_statement(inf, block->statements[block->len - 1]);

  if (base != LiteralType_UNKNOWN && base != branch_return_type) {
    Inferer_throw(
        inf, "block is ambiguous. Expected to return '%s' but got '%s'",
        literal_type_to_str(base), literal_type_to_str(branch_return_type));
  }
  return branch_return_type;
}

// all branches should infer to the same type
LiteralType infer_tree_statement(Inferer *inf, TreeStatement *tree) {
  if (tree->branches.size == 0) {
    return LiteralType_void;
  }

  LiteralType base = -1;
  for (int i = 0; i < tree->branches.size; i++) {
    Branch *b = vector_at(&tree->branches, i);
    printf("\ninfer branch #%d\n of", i);
    dbg_stmt(b->body);
    if (b->body == NULL) {
      Inferer_throw(inf, "null body");
    }
    printf("\n");
    printf("\n-done--\n");
    LiteralType body_return_type = infer_statement(inf, b->body);
    if (base != -1 && base != body_return_type) {
      Inferer_throw(
          inf, "cannot converge tree statement. Tree might return %s or %s",
          literal_type_to_str(base), literal_type_to_str(body_return_type));
    }

    base = body_return_type;
  }

  return base;
}

LiteralType infer_block_statement(Inferer *inf, BlockStatement *block) {
  return infer_block_statement_with_initial(inf, block, LiteralType_UNKNOWN);
}

LiteralType infer_bin_op(Inferer *inf, Statement *stmt_a, Statement *stmt_b,
                         OpType op) {
  if (op == OpType_Custom) {
    return infer_statement(inf, stmt_b);
  }

  LiteralType type_a = infer_statement(inf, stmt_a);
  LiteralType type_b = infer_statement(inf, stmt_b);

  if (type_a == LiteralType_UNKNOWN) {
    return type_b;
  }

  if (type_b == LiteralType_UNKNOWN) {
    return type_a;
  }
  if (type_a != type_b) {
    Inferer_throw(inf, "ambigious binary operation between '%s' and '%s'",
                  literal_type_to_str(type_a), literal_type_to_str(type_b));
  }

  return type_a;
}

LiteralType infer_conditional(Inferer *inf, ConditionalStatement *conditional) {
  printf("\nINFER CONDITIONAL\n");
  LiteralType if_type = infer_block_statement(inf, conditional->if_body);
  printf("\nINFERED IF\n");

  LiteralType else_type =
      infer_block_statement_with_initial(inf, conditional->else_body, if_type);

  printf("\nINFERED ELSE\n");

  return if_type;
}

LiteralType infer_def_invoke(Inferer *inf, Statement *stmt) {

  DefSymbol *defsym = Compiler_defsym_get(inf->com, stmt->sym_decl.name);
  // defsym->btype
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
  printf("\nvarsym will be %d\n", varsym->type);
  return varsym->type;
}

LiteralType infer_statement(Inferer *inf, Statement *stmt) {
  inf->current_stmt = stmt;

  switch (stmt->type) {
  case STMT_BLOCK:
    return infer_block_statement(inf, stmt->block);
  case STMT_TREE:
    return infer_tree_statement(inf, stmt->tree);
  case STMT_LITERAL:
    return stmt->literal.type;
  case STMT_BINARY_OP:
    printf("\nBIN OP\n");
    return infer_bin_op(inf, stmt->left, stmt->right,
                        stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return LiteralType_void;
    // return stmt->sym_decl.type;
  case STMT_VARIABLE_READ:
    return infer_var_read(inf, stmt);
  case STMT_DEF_INVOKE:
    return infer_def_invoke(inf, stmt);
  case STMT_CONDITIONAL:
    return infer_conditional(inf, stmt->conditional);
  case STMT_COMMENT:
    return LiteralType_void;
  }

  Inferer_throw(inf, "could not infer type for expression");
}
