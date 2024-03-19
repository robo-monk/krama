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

void BranchLiteral_push_literal(BranchLiteral *base, LiteralType *literal) {
  vector_push(base, literal);
}

BranchLiteral *BranchLiteral_new(LiteralType *lit) {
  Vec *vec = malloc(sizeof(Vec));
  *vec = new_vec(1, sizeof(LiteralType));
  if (lit != NULL) {
    BranchLiteral_push_literal(vec, lit);
  }
  return vec;
}

void *BranchLiteral_merge(BranchLiteral *base, BranchLiteral *other) {
  if (base == NULL || other == NULL) {
    return base;
  }
  for (int i = 0; i < other->size; i++) {
    vector_push(base, vector_at(other, i));
  }
}

void dbg_branch_literal(BranchLiteral *b) {
  printf("\n Branch literal: ");
  if (b == NULL) {
    printf("NULL");
    return;
  }

  for (int i = 0; i < b->size; i++) {
    LiteralType *current_type = vector_at(b, i);
    if (current_type == NULL) {
      printf("(unknown) | ");
    } else {
      printf("%s | ", literal_type_to_str(*current_type));
    }
  }
}

LiteralType BranchLiteral_converge(Inferer *inf, BranchLiteral *b) {
  LiteralType discovered_literal = -1;
  for (int i = 0; i < b->size; i++) {
    LiteralType *current_type = vector_at(b, i);
    if (current_type == NULL)
      continue;
    if (discovered_literal != -1 && (discovered_literal != *current_type)) {
      Inferer_throw(inf,
                    "could not converge branch literal. ambigious return "
                    "types (%s, %s)",
                    literal_type_to_str(discovered_literal),
                    literal_type_to_str(*current_type));
    }
    discovered_literal = *current_type;
  }

  if (discovered_literal == -1) {
    Inferer_throw(inf, "could not converge branch literal");
  }
  return discovered_literal;
}

BranchLiteral *infer_block_statement_with_initial(Inferer *inf,
                                                  BlockStatement *block,
                                                  BranchLiteral *base) {

  for (int i = 0; i < block->len; i++) {
    // TODO
    // if statement has potential to return
    // infer its type
    // if its different throw error that block statement has ambigious
    // return type

    // for now only infer last stmt in block
    if (i == block->idx) {
      BranchLiteral *branch = infer_statement(inf, block->statements[i]);
      printf("\nMERGING\n");
      BranchLiteral_merge(base, branch);
    }
  }
  return base;
}

BranchLiteral *infer_block_statement(Inferer *inf, BlockStatement *block) {
  return infer_block_statement_with_initial(inf, block,
                                            BranchLiteral_new(NULL));
}

BranchLiteral *infer_bin_op(Inferer *inf, BranchLiteral *type_a,
                            BranchLiteral *type_b, OpType op) {

  // printf("\nINFERING BIN OP:\n");
  // dbg_branch_literal(type_a);
  // printf("\n");
  // dbg_branch_literal(type_b);
  // printf("\n----\n");
  if (BranchLiteral_converge(inf, type_a) !=
      BranchLiteral_converge(inf, type_b)) {
    Inferer_throw(inf, "ambigious binary operation");
  }

  return type_a;
}

BranchLiteral *infer_conditional(Inferer *inf,
                                 ConditionalStatement *conditional) {
  BranchLiteral *if_type = infer_block_statement(inf, conditional->if_body);
  BranchLiteral *else_type =
      infer_block_statement_with_initial(inf, conditional->else_body, if_type);
  dbg_branch_literal(else_type);
  return if_type;
}

BranchLiteral *infer_def_invoke(Inferer *inf, Statement *stmt) {

  DefSymbol *defsym = Compiler_defsym_get(inf->com, stmt->sym_decl.name);
  // defsym->btype
  if (defsym == NULL) {
    Inferer_throw(inf, "did not find symbol %s", stmt->sym_decl.name);
  }

  if (defsym->btype == NULL) {
    // Inferer_throw(inf, "symbol %s is recursive", stmt->sym_decl.name);
  }

  // printf("\ninfered to ->> %s", literal_type_to_str(defsym->type));
  // return new_for_literal(defsym->type);
  return defsym->btype;
}

BranchLiteral *infer_var_read(Inferer *inf, Statement *stmt) {
  VariableSymbol *varsym = Compiler_varsym_get(inf->com, stmt->sym_decl.name);
  if (varsym == NULL) {
    Inferer_throw(inf, "did not find varsymbol %s", stmt->sym_decl.name);
  }
  return BranchLiteral_new(&varsym->type);
}

BranchLiteral *infer_statement(Inferer *inf, Statement *stmt) {
  inf->current_stmt = stmt;

  switch (stmt->type) {
  case STMT_BLOCK:
    return infer_block_statement(inf, stmt->block);
    break;
  case STMT_LITERAL:
    return BranchLiteral_new(&stmt->literal.type);
  case STMT_BINARY_OP:
    return infer_bin_op(inf, infer_statement(inf, stmt->left),
                        infer_statement(inf, stmt->right),
                        stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return infer_statement(inf, stmt->right);

  case STMT_VARIABLE_READ:
    return infer_var_read(inf, stmt);

  case STMT_DEF_INVOKE:
    return infer_def_invoke(inf, stmt);
  case STMT_CONDITIONAL:
    return infer_conditional(inf, stmt->conditional);
  }

  Inferer_throw(inf, "could not infer type for expression");
}

/*
def fib(a: i32) {
    if (a < 2) {
        1
    } else {
        :fib(a-1) + :fib(a-2)
    }
}

def fib2(a: i32) {
    if (a > 2) {
        :fib(a-1) + :fib(a-2)
    } else {
        1
    }
}

def as_float(a: f64) {
    a
}

def as_int(a: i32) {
    a
}


def main() {
    if (0) {
        :as_float(3)
    } else {
        :as_float(3)
    }
}

*/
