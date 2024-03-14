#include "./AbstractSyntaxTree.h"
#include "stdio.h"
#include "stdlib.h"

Statement *new_stmt(StatementType type, Statement *left, Statement *right,
                    Token token) {
  Statement *s = malloc(sizeof(Statement));
  s->type = type;
  s->left = left;
  s->right = right;
  s->token = token;
  return s;
}

Statement *new_sym_decl_stmt(StatementType stmt_type, LiteralType lit_type,
                             string name, Statement *expr, Token token) {
  Statement *s = new_stmt(stmt_type, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = lit_type;
  return s;
}

Statement *new_impl_decl_stmt(LiteralType type, string name, Statement *expr,
                              Token token) {
  Statement *s = new_sym_decl_stmt(IMPL_DECL, LiteralType_i32,
                                   token.value.str_value, expr, token);
  return s;
}

Statement *new_impl_call_stmt(LiteralType type, string name, Statement *args,
                              Token token) {

  Statement *s = new_sym_decl_stmt(IMPL_CALL, LiteralType_i32,
                                   token.value.str_value, args, token);
  return s;
}

Statement *new_var_decl_stmt(LiteralType type, string name, Statement *expr,
                             Token token) {
  Statement *s = new_stmt(VARIABLE_DECL, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_var_read_stmt(LiteralType type, string name, Token token) {
  Statement *s = new_stmt(VARIABLE_READ, NULL, NULL, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_var_write_stmt(LiteralType type, string name, Statement *expr,
                              Token token) {

  Statement *s = new_stmt(VARIABLE_WRITE, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right,
                             Token token) {
  Statement *s = new_stmt(BIN_OP, left, right, token);
  s->bin_op.op = op;
  return s;
}

Statement *new_i32_literal_stmt(int value, Token token) {
  Statement *s = new_stmt(LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_i32;
  s->literal.i32_value = value;
  return s;
}

Statement *new_f64_literal_stmt(double value, Token token) {
  Statement *s = new_stmt(LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_f64;
  s->literal.f64_value = value;
  return s;
}

void _alloc_statement_array(BlockStatement *program) {
  if (program->len < program->max_len) {
    return;
  }

  program->max_len = program->max_len * 2;
  Statement **new_mem =
      realloc(program->statements, sizeof(Statement) * program->max_len);

  if (new_mem == NULL) {
    return;
  }
  program->statements = new_mem;
}

#define DEFAULT_PROGRAM_LEN 1;
BlockStatement *new_block_stmt() {
  BlockStatement *program = malloc(sizeof(BlockStatement));
  program->idx = 0;
  program->len = 0;
  program->max_len = DEFAULT_PROGRAM_LEN;
  program->arg_len = 0;
  program->statements = malloc(sizeof(Statement) * program->max_len);
  return program;
}

void push_stmt_to_block(Statement *stmt, BlockStatement *program) {
  program->len += 1;
  // optionally alloc more memory
  _alloc_statement_array(program);
  program->statements[program->len - 1] = stmt;
}

Statement *new_conditional_stmt(Statement *condition, BlockStatement *if_body,
                                BlockStatement *else_body, Token token) {
  Statement *s = new_stmt(STMT_CONDITIONAL, NULL, NULL, token);
  s->conditional = malloc(sizeof(ConditionalStatement));
  s->conditional->if_body = if_body;
  s->conditional->else_body = else_body;
  s->conditional->condition = condition;
  return s;
}

#include <stdio.h>
#include <string.h>

void dbg_op(OpType op) { printf(" X "); }

void print_indented(const char *text, int indent) {
  char indentation[256]; // Adjust size as needed, considering maximum expected
                         // indentation depth
  memset(indentation, '\t', indent); // Fill the indentation array with tabs
  indentation[indent] = '\0';        // Null-terminate the string

  // Print each line of the input text with indentation
  const char *lineStart = text;
  const char *lineEnd;
  while ((lineEnd = strchr(lineStart, '\n')) != NULL) {
    printf("%s", indentation);
    fwrite(lineStart, 1, lineEnd - lineStart + 1,
           stdout); // +1 to include the newline
    lineStart = lineEnd + 1;
  }
  // Print any remaining text that doesn't end in a newline
  if (*lineStart) {
    printf("%s%s\n", indentation, lineStart);
  }
}

void dbg_stmt_with_indent(const Statement *stmt, int indent);

void dbg_block_stmt_with_indent(const BlockStatement *block, int indent) {
  for (int i = 0; i < block->len; i++) {
    dbg_stmt_with_indent(block->statements[i], indent + 1);
  }
}

void dbg_stmt_with_indent(const Statement *stmt, int indent) {
  if (!stmt)
    return;

  char buffer[1024]; // Adjust size as needed, considering maximum expected
                     // output length

  switch (stmt->type) {
  case BIN_OP:
    snprintf(buffer, sizeof(buffer), "Statement Type: Binary Operation\n");
    // dbg_token(stmt->token); // Note: You may need to adjust dbg_token to work
    // with this approach
    break;
  case LITERAL:
    // printf("\n");
    // dbg_token(stmt->token);
    // printf("\n");
    snprintf(buffer, sizeof(buffer), "Statement Type: Literal with type");
    // Append specific literal type and value to buffer here
    break;
  case VARIABLE_DECL:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Variable Declaration: Type , Name: %s\n",
             stmt->sym_decl.name);
    break;
  case VARIABLE_WRITE:
    snprintf(buffer, sizeof(buffer), "Statement Type: Variable Write: %s\n",
             stmt->sym_decl.name);
    break;
  case VARIABLE_READ:
    snprintf(buffer, sizeof(buffer), "Statement Type: Variable Read: %s\n",
             stmt->sym_decl.name);

    break;
  case IMPL_DECL:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Implementation Decl: %s\n", stmt->sym_decl.name);
    break;
  case IMPL_CALL:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Implementation Call: %s\n", stmt->sym_decl.name);
    break;
  case BLOCK:
    dbg_block_stmt_with_indent(stmt->block, indent + 1);
    break;
  case STMT_CONDITIONAL:
    // printf("\n\n STMT Type %d", stmt->type);
    snprintf(buffer, sizeof(buffer), "Statement Type: Conditional Stmt\n");
    // printf("    [CONDITION]:\n");
    dbg_stmt_with_indent(stmt->conditional->condition, indent + 1);
    // printf("    [BODY]:\n");
    dbg_block_stmt_with_indent(stmt->conditional->if_body, indent + 1);
    break;
  }

  print_indented(buffer, indent);

  // Recursively print left and right child statements with increased
  // indentation
  if (stmt->left) {
    dbg_stmt_with_indent(stmt->left, indent + 1);
  }
  if (stmt->right) {
    dbg_stmt_with_indent(stmt->right, indent + 1);
  }
}

// Wrapper function remains the same
void dbg_stmt(const Statement *stmt) { dbg_stmt_with_indent(stmt, 0); }
