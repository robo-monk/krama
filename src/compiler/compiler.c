#include "compiler.h"
#include "../frontend/LiteralType.h"
#include "inference.h"
#include "stdio.h"
#include "stdlib.h"

string compile_block_statement(Compiler *com, BlockStatement *stmt) {
  StrVec statements = new_str_vec(4);
  for (int i = 0; i < stmt->len; i++) {
    printf("\nCOMPILE BLOCK STATEMENT %d\n", i);
    str_vector_push(&statements, com_statement(com, stmt->statements[i]));
    str_vector_push(&statements, ";\n");
  }

  string a = str_vector_join(&statements);
  printf("\n\nvector join is %s\n\n", a);
  Compiler_impl_push(com, a);
  return a;
}

string compile_write_variable(Compiler *com, string var_name,
                              LiteralType var_type, string value) {

  VariableSymbol *varsym = Compiler_varsym_get(com, var_name);
  if (varsym == NULL) {
    Compiler_throw(com, "tried to write an undeclared variable '%s'", var_name);
  }
  return concat(3, var_name, " = ", value);
}

string com_declare_variable(Compiler *com, string var_name,
                            LiteralType var_type, string value) {
  Compiler_varsym_declare(com, var_name, var_type);

  return concat(3, literal_type_to_str(var_type), " ",
                compile_write_variable(com, var_name, var_type, value));
}

string compile_read_variable(Compiler *com, string var_name,
                             LiteralType var_type) {
  VariableSymbol *varsym = Compiler_varsym_get(com, var_name);
  if (varsym == NULL) {
    Compiler_throw(com, "tried to read from undeclared variable '%s'",
                   var_name);
  }
  return concat(4, "(", literal_type_to_str(var_type), ")", var_name);
}

string compile_call_symbol(Compiler *com, Statement *stmt) {
  DefSymbol *defsym = Compiler_defsym_get(com, stmt->sym_decl.name);
  if (defsym == NULL) {
    Compiler_throw(com, "tried to invoke undeclared definition '%s'",
                   stmt->sym_decl.name);
  }

  printf("\n-----\n");
  dbg_stmt(stmt);
  printf("\n-----\n");
  // printf("\nCOMPIKLING %d ARG\n", stmt->block->arg_len);
  Statement *args_statement = stmt->right;
  if (args_statement->type != STMT_BLOCK) {
    // Compiler_throw("expected block statement that defines arguments");
    Compiler_throw(com, "expected block statement that defines arguments");
  }

  StrVec arguments = new_str_vec(1);
  for (int i = 0; i < args_statement->block->len; i++) {
    if (i != 0) {
      str_vector_push(&arguments, ",");
    }
    string arg_value = com_statement(com, args_statement->block->statements[i]);
    str_vector_push(&arguments, arg_value);
  };
  return concat(4, stmt->sym_decl.name, "(", str_vector_join(&arguments), ")");
}

string com_bin_op(string left, string right, OpType optype) {
  return concat(3, left, optype_to_str(optype), right);
}

string com_conditional(Compiler *com, ConditionalStatement *conditional) {
  string else_com = "";
  if (conditional->else_body) {
    else_com =
        concat(3, " else {\n",
               compile_block_statement(com, conditional->else_body), "}");
  }

  return concat(6, "if (", com_statement(com, conditional->condition), ") {\n",
                compile_block_statement(com, conditional->if_body), "}",
                else_com);
}

string compile_block_arguments(Compiler *com, BlockStatement *block) {
  // string compiled_args = malloc(sizeof(char) * 512);
  StrVec compiled_args = new_str_vec(1);
  for (int i = 0; i < block->arg_len; i++) {
    if (i != 0) {
      str_vector_push(&compiled_args, ", ");
    }

    Compiler_varsym_declare(com, block->args[i]->name, block->args[i]->type);
    string arg_str = concat(3, literal_type_to_str(block->args[i]->type), " ",
                            block->args[i]->name);

    // com_
    str_vector_push(&compiled_args, arg_str);
  }

  return str_vector_join(&compiled_args);
}

string com_def_declaration(Compiler *com, string def_name, Statement *stmt) {
  if (Compiler_defsym_get(com, def_name)) {
    Compiler_throw(com, "redecleration of definition'%s'", def_name);
  }

  Compiler scoped_compiler = Compiler_new();
  scoped_compiler.upper = com;
  string compiled_args = compile_block_arguments(&scoped_compiler, stmt->block);

  Inferer inf = Inferer_new(&scoped_compiler);
  LiteralType return_type = infer_statement(&inf, stmt);
  printf("\ninfered defsym '%s' to %s\n", def_name,
         literal_type_to_str(return_type));

  Compiler_defsym_declare(com, def_name, stmt, return_type);

  printf("\n -- compiled args %s\n", compiled_args);
  string decleration_str = concat(6, literal_type_to_str(return_type), " ",
                                  def_name, "(", compiled_args, ")");

  Compiler_decl_push(com, decleration_str);
  string body = compile_block_statement(&scoped_compiler, stmt->block);
  Compiler_free(&scoped_compiler);
  return concat(4, decleration_str, " {\n", body, "\n}");
}

string com_statement(Compiler *com, Statement *stmt) {
  com->current_stmt = stmt;
  switch (stmt->type) {
  case STMT_BLOCK:
    return compile_block_statement(com, stmt->block);
  case STMT_LITERAL:
    return concat(4, "(", literal_type_to_str(stmt->literal.type), ")",
                  literal_val2str(stmt->literal));
  case STMT_BINARY_OP:
    return com_bin_op(com_statement(com, stmt->left),
                      com_statement(com, stmt->right),
                      stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return com_declare_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                                com_statement(com, stmt->right));

  case STMT_VARIABLE_READ:
    return compile_read_variable(com, stmt->sym_decl.name, stmt->sym_decl.type);
  case STMT_VARIABLE_WRITE:
    return compile_write_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                                  com_statement(com, stmt->right));

  case STMT_DEF_INVOKE:
    return compile_call_symbol(com, stmt);
  case STMT_CONDITIONAL:
    // printf("\neval conditional:\n");
    // dbg_stmt(stmt);
    return com_conditional(com, stmt->conditional);
  case STMT_DEF_DECL:
    return com_def_declaration(com, stmt->sym_decl.name, stmt->right);
  }

  // report_syntax_error(stmt->token, "unsupported token");
  Compiler_throw(com, "unsupportd token");
}

void compile_program(BlockStatement *program, char *filename) {
  Compiler com = Compiler_new();
  compile_block_statement(&com, program);
  Compiler_write_to_file(&com, filename);
}
