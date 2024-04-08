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

Statement *TreeStatement_new(string name, Token token, Vec args) {
  Statement *s = new_stmt(STMT_TREE, NULL, NULL, token);
  s->tree = malloc(sizeof(TreeStatement));
  s->tree->name = name;
  s->tree->arguments = args;
  s->tree->branches = new_vec(1, sizeof(Branch));
  return s;
}

void TreeStatement_push(TreeStatement *tree, Branch *branch) {
  vector_push(&tree->branches, branch);
}

Branch *Branch_new(Statement *body, Statement *condition) {
  Branch *b = malloc(sizeof(Branch));
  b->body = body;
  b->condition = condition;
  return b;
}

Statement *new_comment_stmt(Token token) {
  return new_stmt(STMT_COMMENT, NULL, NULL, token);
}

Statement *new_sym_decl_stmt(StatementType stmt_type, LiteralType lit_type,
                             string name, Statement *expr, Token token) {
  Statement *s = new_stmt(stmt_type, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = lit_type;
  return s;
}

Statement *new_impl_decl_stmt(string name, Statement *expr, Token token) {
  Statement *s = new_sym_decl_stmt(STMT_DEF_DECL, LiteralType_UNKNOWN,
                                   token.value.str_value, expr, token);
  return s;
}

Statement *new_impl_call_stmt(LiteralType type, string name,
                              BlockStatement *args, Token token) {
  Statement *arg_stmt = new_stmt(STMT_BLOCK, NULL, NULL, token);
  arg_stmt->block = args;
  Statement *s = new_sym_decl_stmt(STMT_DEF_INVOKE, LiteralType_i32,
                                   token.value.str_value, arg_stmt, token);
  return s;
}

Statement *new_var_decl_stmt(LiteralType type, string name, Statement *expr,
                             Token token) {
  Statement *s = new_stmt(STMT_VARIABLE_DECL, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_var_read_stmt(LiteralType type, string name, Token token) {
  Statement *s = new_stmt(STMT_VARIABLE_READ, NULL, NULL, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_var_write_stmt(LiteralType type, string name, Statement *expr,
                              Token token) {

  Statement *s = new_stmt(STMT_VARIABLE_WRITE, NULL, expr, token);
  s->sym_decl.name = name;
  s->sym_decl.type = type;
  return s;
}

Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right,
                             Token token) {
  Statement *s = new_stmt(STMT_BINARY_OP, left, right, token);
  s->bin_op.op = op;
  return s;
}

Statement *new_numerical_literal_stmt(long value, Token token) {
  Statement *s = new_stmt(STMT_LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_NUMERAL;
  s->literal.i32_value = value;
  return s;
}

Statement *new_char_literal_stmt(char c, Token token) {
  Statement *s = new_stmt(STMT_LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_char;
  s->literal.cha_value = c;
  return s;
}

Statement *new_i32_literal_stmt(int value, Token token) {
  Statement *s = new_stmt(STMT_LITERAL, NULL, NULL, token);
  s->literal.type = LiteralType_i32;
  s->literal.i32_value = value;
  return s;
}

Statement *new_f64_literal_stmt(double value, Token token) {
  Statement *s = new_stmt(STMT_LITERAL, NULL, NULL, token);
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
  program->len = 0;
  program->max_len = DEFAULT_PROGRAM_LEN;
  program->arg_len = 0;
  program->statements = malloc(sizeof(Statement) * program->max_len);
  return program;
}

void push_stmt_to_block(Statement *stmt, BlockStatement *program) {
  if (!stmt) {
    // printf("\n pushing null block??\n");
    // exit(1);
    return;
  }
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
    printf("\n NULL STMT \n");
  return;

  char buffer[1024]; // Adjust size as needed, considering maximum expected
                     // output length

  switch (stmt->type) {
  case STMT_BINARY_OP:
    snprintf(buffer, sizeof(buffer), "Statement Type: Binary Operation\n");
    dbg_token(stmt->token); // Note: You may need to adjust dbg_token to work
    printf("\n");
    // with this approach
    break;
  case STMT_LITERAL:
    // printf("\n");
    // dbg_token(stmt->token);
    // printf("\n");
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Literal with type and value %d",
             stmt->token.value.i32_value);
    // Append specific literal type and value to buffer here
    break;
  case STMT_VARIABLE_DECL:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Variable Declaration: Type , Name: %s\n",
             stmt->sym_decl.name);
    break;
  case STMT_VARIABLE_WRITE:
    snprintf(buffer, sizeof(buffer), "Statement Type: Variable Write: %s\n",
             stmt->sym_decl.name);
    break;
  case STMT_VARIABLE_READ:
    snprintf(buffer, sizeof(buffer), "Statement Type: Variable Read: %s\n",
             stmt->sym_decl.name);

    break;
  case STMT_DEF_DECL:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Implementation Decl: %s\n", stmt->sym_decl.name);
    break;
  case STMT_DEF_INVOKE:
    snprintf(buffer, sizeof(buffer),
             "Statement Type: Implementation Call: %s\n", stmt->sym_decl.name);
    break;
  case STMT_BLOCK:
    dbg_block_stmt_with_indent(stmt->block, indent + 1);
    break;
  case STMT_TREE:
    snprintf(buffer, sizeof(buffer), "Statement Type: Tree Stmt\n");
  case STMT_CONDITIONAL:
    // printf("\n\n STMT Type %d", stmt->type);
    snprintf(buffer, sizeof(buffer), "Statement Type: Conditional Stmt\n");
    // printf("    [CONDITION]:\n");
    dbg_stmt_with_indent(stmt->conditional->condition, indent + 1);
    // printf("    [BODY]:\n");
    dbg_block_stmt_with_indent(stmt->conditional->if_body, indent + 1);
    break;
  case STMT_COMMENT:
    snprintf(buffer, sizeof(buffer), "Statement Type: Comment: %s\n",
             stmt->token.value.str_value);
    break;
  }

  print_indented(buffer, indent);

  // Recursively print left and right child statements with increased
  // indentation
  if (stmt->left != NULL) {
    snprintf(buffer, sizeof(buffer), "Left: ");
    dbg_stmt_with_indent(stmt->left, indent + 1);
  }
  if (stmt->right != NULL) {
    snprintf(buffer, sizeof(buffer), "Right: ");
    dbg_stmt_with_indent(stmt->right, indent + 1);
  }
}

// Wrapper function remains the same
void dbg_stmt(const Statement *stmt) { dbg_stmt_with_indent(stmt, 0); }
