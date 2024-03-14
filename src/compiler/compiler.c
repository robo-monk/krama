#include "compiler.h"
#include "../frontend/LiteralType.h"
#include "stdio.h"
#include "stdlib.h"

CCompiler new_ccompiler() {
  return (CCompiler){.headers = new_vec(1024, sizeof(string)),
                     .implementations = new_vec(1024, sizeof(string))};
}

void push_implementation(CCompiler *com, string impl_str) {
  vector_push(&com->implementations, impl_str);
}
void push_header(CCompiler *com, string head_str) {
  vector_push(&com->implementations, head_str);
}

void throw_compilation_error(string error) {
  printf("\n[compiler] Error! %s\n", error);
  exit(1);
}

// StrVec com_block_statement(CCompiler *com, BlockStatement *stmt);
string com_block_statement(CCompiler *com, BlockStatement *stmt) {
  StrVec statements = new_str_vec(512);
  for (int i = 0; i < stmt->len; i++) {
    string comstmt = com_statement(com, stmt->statements[i]);
    str_vector_push(&statements, comstmt);
  }
  string a = str_vector_join(&statements);
  printf("\n\nvector join is %s\n\n", a);
  push_implementation(com, a);
  return a;
}

string com_declare_variable(CCompiler *com, string var_name,
                            LiteralType var_type, string value) {
  return concat(6, literal_type_to_str(var_type), " ", var_name, " = ", value,
                ";\n");
}

string com_statement(CCompiler *com, Statement *stmt) {
  switch (stmt->type) {
  case STMT_BLOCK:
    return com_block_statement(com, stmt->block);
  case STMT_LITERAL:
    return concat(4, "(", literal_type_to_str(stmt->literal.type), ")",
                  literal_val2str(stmt->literal));
  // case STMT_BINARY_OP:
  // return com_bin_op(com_statement(com, stmt->left),
  //                   com_statement(ipr, stmt->right),
  //                   stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return com_declare_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                                com_statement(com, stmt->right));

    // case STMT_VARIABLE_READ:
    //   return com_read_variable(com, stmt->sym_decl.name,
    //   stmt->sym_decl.type);
    // case STMT_VARIABLE_WRITE:
    //   return com_write_variable(com, stmt->sym_decl.name,
    //   stmt->sym_decl.type,
    //                             evaluate_statement(com, stmt->right));
    // case STMT_DEF_DECL:
    //   return com_declare_implementation(ipr, stmt->sym_decl.name,
    //                                     stmt->sym_decl.type, stmt->right);
    // case STMT_DEF_INVOKE:
    //   return com_call_symbol(com, stmt);
    // case STMT_CONDITIONAL:
    //   // printf("\neval conditional:\n");
    //   // dbg_stmt(stmt);
    //   return com_conditional_statement(ipr, stmt->conditional);
  }

  throw_compilation_error("found unsupported token while compiling");
}

void write_ccompiler_state_to_file(CCompiler *com, string filename) {
  FILE *file =
      fopen(filename,
            "w"); // Open the file for writing ("w" mode overwrites the file)
  if (file == NULL) {
    perror("Failed to open file"); // Print the error message to stderr
    exit(1);
  }

  fprintf(file, "// headers \n");
  for (int i = 0; i < com->headers.size; i++) {
    fprintf(file, "%s\n", (string)vector_at(&com->headers, i));
  }

  fprintf(file, "\n\n // implementations \n");
  for (int i = 0; i < com->implementations.size; i++) {
    fprintf(file, "%s\n", (string)vector_at(&com->implementations, i));
  }
  // Write the text to the file, followed by a newline character
  fclose(file); // Close the file
}
void compile_program(BlockStatement *program, char *filename) {
  CCompiler com = new_ccompiler();
  com_block_statement(&com, program);
  write_ccompiler_state_to_file(&com, filename);
}
