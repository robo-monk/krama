#include "inference.h"
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

void dbg_branch_literal(BranchLiteral *b) {
  printf("\n Branch literal: ");
  BranchLiteral *upper = b;
  while (upper != NULL) {
    printf("%s | ", literal_type_to_str(upper->type));
    upper = upper->branch;
  }
}

LiteralType converge_branch_literal(Inferer *inf, BranchLiteral *b) {
  LiteralType discovered_literal = -1;
  BranchLiteral *current = b;
  while (current != NULL) {
    if (current->type != -1) {
      if (discovered_literal != -1 && (discovered_literal != current->type)) {
        Inferer_throw(inf,
                      "could not converge branch literal. ambigious return "
                      "types (%s, %s)",
                      literal_type_to_str(discovered_literal),
                      literal_type_to_str(current->type));
      }
      discovered_literal = current->type;
    }
    current = current->branch;
  }
  if (discovered_literal == -1) {
    Inferer_throw(inf, "could not converge branch literal");
  }
  return discovered_literal;
}

BranchLiteral new_for_literal(LiteralType lit) {
  return (BranchLiteral){.type = lit, .branch = NULL};
}

BranchLiteral *BranchLiteral_push_branch(BranchLiteral *base,
                                         BranchLiteral *branch) {
  if (base->branch == NULL) {
    base->branch = branch;
    return base;
  }
  return BranchLiteral_push_branch(base->branch, branch);
}

BranchLiteral
infer_block_statement_with_initial(Inferer *inf, BlockStatement *block,
                                   BranchLiteral *branch_literal) {

  for (int i = 0; i < block->len; i++) {
    // TODO
    // if statement has potential to return
    // infer its type
    // if its different throw error that block statement has ambigious
    // return type

    // for now only infer last stmt in block
    if (i == block->idx) {
      BranchLiteral branch = infer_statement(inf, block->statements[i]);
      BranchLiteral *bb = malloc(sizeof(BranchLiteral));
      bb->branch = branch.branch;
      bb->type = branch.type;
      BranchLiteral_push_branch(branch_literal, bb);
    }
  }
  return *branch_literal;
}

BranchLiteral infer_block_statement(Inferer *inf, BlockStatement *block) {
  BranchLiteral *previous_return_type = malloc(sizeof(BranchLiteral));
  previous_return_type->branch = NULL;
  previous_return_type->type = -1;
  return infer_block_statement_with_initial(inf, block, previous_return_type);
}

BranchLiteral infer_bin_op(Inferer *inf, BranchLiteral type_a,
                           BranchLiteral type_b, OpType op) {

  printf("\nINFERING BIN OP:\n");
  dbg_branch_literal(&type_a);
  printf("\n");
  dbg_branch_literal(&type_b);
  printf("\n----\n");
  // if (converge_branch_literal(inf, &type_a) !=
  //     converge_branch_literal(inf, &type_b)) {
  //   Inferer_throw(inf, "ambgious binary operation");
  // }

  return type_a;
}

BranchLiteral infer_conditional(Inferer *inf,
                                ConditionalStatement *conditional) {
  BranchLiteral if_type = infer_block_statement(inf, conditional->if_body);
  printf("\nif type branch literal: ");
  dbg_branch_literal(&if_type);

  printf("\nelse type branch literal: ");

  BranchLiteral else_type =
      infer_block_statement_with_initial(inf, conditional->else_body, &if_type);
  dbg_branch_literal(&else_type);

  // if (converge_branch_literal(inf, &if_type) !=
  //     converge_branch_literal(inf, &else_type)) {
  //   // return (BranchLiteral){.branch = &else_type,
  //   //                        .type = converge_branch_literal(inf,
  //   &if_type)}; Inferer_throw(inf,
  //                 "ambgious conditional. If statement returns (%s) else
  //                 stmt " "returns(% s) ",
  //                 literal_type_to_str(if_type.type),
  //                 literal_type_to_str(else_type.type));
  // }

  return if_type;
}

BranchLiteral infer_def_invoke(Inferer *inf, Statement *stmt) {

  printf("\nINFERING DEF INVOKE '%s' \n", stmt->sym_decl.name);
  DefSymbol *defsym = Compiler_defsym_get(inf->com, stmt->sym_decl.name);
  if (defsym == NULL) {
    Inferer_throw(inf, "did not find symbol %s", stmt->sym_decl.name);
  }

  // printf("\ninfered to ->> %s", literal_type_to_str(defsym->type));
  // return new_for_literal(defsym->type);
  return defsym->btype;
}

BranchLiteral infer_var_read(Inferer *inf, Statement *stmt) {
  printf("\nINFERING VARIABLE READ\n");
  VariableSymbol *varsym = Compiler_varsym_get(inf->com, stmt->sym_decl.name);
  if (varsym == NULL) {
    Inferer_throw(inf, "did not find varsymbol %s", stmt->sym_decl.name);
  }
  printf("\ninfered to ->> %s", literal_type_to_str(varsym->type));

  return new_for_literal(varsym->type);
}

BranchLiteral infer_statement(Inferer *inf, Statement *stmt) {
  inf->current_stmt = stmt;
  switch (stmt->type) {
  case STMT_BLOCK:
    return infer_block_statement(inf, stmt->block);
    break;
  //   return compile_block_statement(com, stmt->block);
  case STMT_LITERAL:
    printf("\n ----- reach base case for inference '%s'",
           literal_type_to_str(stmt->literal.type));
    // return stmt->literal.type;
    return new_for_literal(stmt->literal.type);
  case STMT_BINARY_OP:
    return infer_bin_op(inf, infer_statement(inf, stmt->left),
                        infer_statement(inf, stmt->right),
                        stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return infer_statement(inf, stmt->right);

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
