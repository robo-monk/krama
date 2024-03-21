#ifndef H_AST
#define H_AST

#include "../frontend/tokeniser.h"
#include "../utils.h"

typedef struct BlockStatement BlockStatement;
typedef struct Statement Statement;
// typedef struct BranchLiteral BranchLiteral;

typedef enum {
  STMT_BLOCK,
  STMT_BINARY_OP,
  STMT_LITERAL,

  STMT_VARIABLE_DECL,
  STMT_VARIABLE_WRITE,
  STMT_VARIABLE_READ,

  STMT_DEF_DECL,
  STMT_DEF_INVOKE,

  STMT_CONDITIONAL,
  STMT_COMMENT
} StatementType;

typedef enum {
  LiteralType_i32,
  LiteralType_f64,
  LiteralType_char,
  LiteralType_bool,
  LiteralType_void,
  LiteralType_ptr
} LiteralType;

typedef struct {
  LiteralType type;
  union {
    int i32_value;
    double f64_value;
    char cha_value;
  };
} LiteralStatement;

typedef struct {
  OpType op;
} BinExpressionStatement;

typedef struct {
  LiteralType type;
  LiteralType obj;
  string name;
} SymbolStatement;

typedef struct {
  SymbolStatement **symbols;
  unsigned int length;
} SymbolStatements;

typedef SymbolStatement Argument;

// typedef struct {
//   BlockStatement *stmts;
// } BlockStatement;

struct BlockStatement {
  Argument **args;
  unsigned int arg_len;

  Statement **statements;
  unsigned int len;
  unsigned int max_len;
};

typedef struct {
  Statement *condition;
  BlockStatement *if_body;
  BlockStatement *else_body;
} ConditionalStatement;

// todo all of them should be pointers
struct Statement {
  union {
    LiteralStatement literal;
    BinExpressionStatement bin_op;
    SymbolStatement sym_decl;
    BlockStatement *block;
    ConditionalStatement *conditional;
  };
  StatementType type;
  Statement *left;
  Statement *right;

  Token token;
};

BlockStatement *new_block_stmt();
void push_stmt_to_block(Statement *stmt, BlockStatement *block_stmt);

Statement *new_stmt(StatementType type, Statement *left, Statement *right,
                    Token token);

Statement *new_comment_stmt(Token token);

Statement *new_conditional_stmt(Statement *condition, BlockStatement *if_body,
                                BlockStatement *else_body, Token token);

Statement *new_bin_expr_stmt(OpType op, Statement *left, Statement *right,

                             Token token);
Statement *new_i32_literal_stmt(int value, Token token);
Statement *new_f64_literal_stmt(double value, Token token);
Statement *new_var_read_stmt(LiteralType type, string name, Token token);
Statement *new_var_write_stmt(LiteralType type, string name, Statement *expr,
                              Token token);
Statement *new_var_decl_stmt(LiteralType type, string name, Statement *expr,
                             Token token);

Statement *new_impl_decl_stmt(LiteralType type, string name, Statement *expr,
                              Token token);

Statement *new_impl_call_stmt(LiteralType type, string name,
                              BlockStatement *args, Token token);
void dbg_stmt(const Statement *stmt);
#endif
