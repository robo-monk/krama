#include "compiler.h"
#include "../frontend/LiteralType.h"
#include "CCompiler.h"
#include "inference.h"
#include "stdio.h"
#include "stdlib.h"

bool is_returnable_stmt(Statement *stmt) {
  switch (stmt->type) {
  case STMT_VARIABLE_READ:
  case STMT_LITERAL:
  case STMT_BINARY_OP:
  case STMT_DEF_INVOKE:
    return true;
  default:
    return false;
  }
}

string compile_block_statement(Compiler *com, BlockStatement *stmt) {
  StrVec statements = new_str_vec(4);
  for (int i = 0; i < stmt->len; i++) {
    printf("\nCOMPILE BLOCK STATEMENT %d\n", i);
    // if its the last index add the return keyword
    // if (i == stmt->len - 1) {
    if (i == stmt->len - 1 && is_returnable_stmt(stmt->statements[i])) {
      str_vector_push(&statements, "return ");
    }
    str_vector_push(&statements, com_statement(com, stmt->statements[i]));
    str_vector_push(&statements, ";\n");
  }

  string a = str_vector_join(&statements);
  Compiler_impl_push(com, a);
  return a;
}

string compile_symbol_statements_vector_as_args_def(Compiler *com, Vec *args) {
  StrVec compiled_args = new_str_vec(1);
  for (int i = 0; i < args->size; i++) {
    if (i != 0) {
      str_vector_push(&compiled_args, ", ");
    }

    SymbolStatement *sym = vector_at(args, i);
    Compiler_varsym_declare(com, sym->name, sym->type);
    string arg_str = concat(3, literal_type_to_str(sym->type), " ", sym->name);
    str_vector_push(&compiled_args, arg_str);
  }

  return str_vector_join(&compiled_args);
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

// potential optimisation: if all statements are "compiler time evaluate", make
// it a swithc statement
string compile_tree_statement(Compiler *com, Statement *stmt) {
  TreeStatement *tree = stmt->tree;

  Compiler scope = Compiler_new();
  scope.upper = com;
  string arg_defs =
      compile_symbol_statements_vector_as_args_def(&scope, &tree->arguments);

  Compiler_defsym_declare(&scope, tree->name, stmt, NULL);

  Inferer inf = Inferer_new(&scope);
  LiteralType return_literal = infer_statement(&inf, stmt);
  printf("-----\n return literal type is %s\n\n",
         literal_type_to_str(return_literal));

  Compiler_defsym_declare(com, tree->name, stmt, return_literal);

  StrVec statements = new_str_vec(tree->branches.size);
  printf("\n arg defs are %s", arg_defs);

  for (int i = 0; i < tree->branches.size; i++) {
    Branch *branch = vector_at(&tree->branches, i);
    string condition = "1"; // default condition

    if (branch->condition != NULL) {
      condition = com_statement(&scope, branch->condition);
    }

    string body = com_statement(&scope, branch->body);
    printf("\n condition is [%s] and body is [%s]", condition, body);

    str_vector_push(&statements,
                    concat(5, "if (", condition, ") {", body, "}"));
  }
  printf("\n -- done -- \n");

  string body = str_vector_join(&statements);
  printf("\n -- body is -- \n %s\n", body);
  string decl = concat(6, literal_type_to_str(return_literal), " ", tree->name,
                       "(", arg_defs, ")");

  Compiler_decl_push(com, decl);
  Compiler_free(&scope);
  string compiled = concat(4, decl, "{", body, "}");

  return compiled;
}

string compile_write_variable(Compiler *com, string var_name,
                              LiteralType var_type, Statement *stmt) {

  VariableSymbol *varsym = Compiler_varsym_get(com, var_name);
  if (varsym == NULL) {
    Compiler_throw(com, "tried to write an undeclared variable '%s'", var_name);
  }

  Inferer inf = Inferer_new(com);
  LiteralType infered_lt = infer_statement(&inf, stmt);

  if (infered_lt == LiteralType_UNKNOWN && var_type == LiteralType_UNKNOWN) {
    Compiler_throw(com, "cannot infer type this time... %s", var_name);
  }

  if (infered_lt != LiteralType_NUMERAL && infered_lt != var_type) {
    Compiler_throw(
        com, "attempted to assign '%s' to '%s' while declaring variable '%s'",
        literal_type_to_str(infered_lt), literal_type_to_str(var_type),
        var_name);
  }

  string compiled_rhs = com_statement(com, stmt);
  return concat(3, var_name, " = ", compiled_rhs);
}

string com_declare_variable(Compiler *com, string var_name,
                            LiteralType var_type, Statement *stmt) {

  if (var_type == LiteralType_UNKNOWN) {
    Inferer inf = Inferer_new(com);
    var_type = infer_statement(&inf, stmt);
  }

  if (var_type == LiteralType_NUMERAL) {
    // programmer has to specify what type of numeral this is, so this will
    // break
    Compiler_throw(com, "specify what time of numeral you want. hint: i64");
  }

  Compiler_varsym_declare(com, var_name, var_type);
  return concat(3, literal_type_to_str(var_type), " ",
                compile_write_variable(com, var_name, var_type, stmt));
}

string compile_read_variable(Compiler *com, string var_name,
                             LiteralType var_type) {
  VariableSymbol *varsym = Compiler_varsym_get(com, var_name);
  if (varsym == NULL) {
    Compiler_throw(com, "tried to read from undeclared variable '%s'",
                   var_name);
  }
  // return concat(4, "(", literal_type_to_str(var_type), ")", var_name);
  return concat(1, var_name);
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

string com_bin_op(Compiler *com, Statement *left, Statement *right,
                  OpType optype) {
  if (optype == OpType_Custom) {
    // printf("\nLEFT: \n");
    // dbg_stmt(left);

    // printf("\nRIGHT: \n");
    // dbg_stmt(right);

    printf("\n compiler.c\n");
    push_stmt_to_block(left, right->right->block);
    if (right->right->type != STMT_BLOCK) {
      Compiler_throw(com, "expected block statement that defines arguments");
    }

    return com_statement(com, right);
  }
  return concat(3, com_statement(com, left), optype_to_str(optype),
                com_statement(com, right));
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

string compile_def_decleration(Compiler *com, string def_name,
                               Statement *stmt) {
  if (Compiler_defsym_get(com, def_name)) {
    Compiler_throw(com, "redecleration of definition'%s'", def_name);
  }

  Compiler scoped_compiler = Compiler_new();
  scoped_compiler.upper = com;
  string compiled_args = compile_block_arguments(&scoped_compiler, stmt->block);

  Compiler_defsym_declare(&scoped_compiler, def_name, stmt,
                          LiteralType_UNKNOWN);

  Inferer inf = Inferer_new(&scoped_compiler);
  string body = compile_block_statement(&scoped_compiler, stmt->block);

  LiteralType return_type = LiteralType_UNKNOWN;
  printf("\n------ *** INFERING def '%s' *** --------\n", def_name);
  return_type = infer_statement(&inf, stmt);
  // printf("\n ACTUALLY RETURNED?? %d", infer_var_read(&inf, stmt));
  printf("\n---- *** INFERED def '%s' TO %s (%d) \n", def_name,
         literal_type_to_str(return_type), return_type);

  // LiteralType return_type = return_type_branch.type;

  // printf("\ninfered defsym '%s' to %s\n", def_name,
  // literal_type_to_str(return_type));

  Compiler_defsym_declare(com, def_name, stmt, return_type);
  printf("\nliteral type unfer\n");

  string decleration_str = concat(6, literal_type_to_str(return_type), " ",
                                  def_name, "(", compiled_args, ")");

  Compiler_decl_push(com, decleration_str);
  // Compiler_free(&scoped_compiler);
  return concat(4, decleration_str, " {\n", body, "\n}");
}

string compile_comment(Compiler *com, Token token_comment) {
  // check if its a magic comment $
  if (token_comment.value.str_value[1] == 'c') {
    int i = 3;
    string c_command = calloc(512, sizeof(char));
    while (token_comment.value.str_value[i] != '\0') {
      c_command[i - 3] = token_comment.value.str_value[i];
      i++;
    }
    return c_command;
  }

  return concat(2, "//", token_comment.value.str_value);
}

string com_statement(Compiler *com, Statement *stmt) {
  com->current_stmt = stmt;
  switch (stmt->type) {
  case STMT_BLOCK:
    return compile_block_statement(com, stmt->block);
  case STMT_TREE:
    return compile_tree_statement(com, stmt);
  case STMT_LITERAL:
    return literal_val2str(stmt->literal);
    // return concat(4, "(", literal_type_to_str(stmt->literal.type), ") ",
    // return concat(4, "(", literal_type_to_str(stmt->literal.type), ") ",
    //               literal_val2str(stmt->literal));
  case STMT_BINARY_OP:
    return com_bin_op(com, stmt->left, stmt->right, stmt->token.value.op_type);
  case STMT_VARIABLE_DECL:
    return com_declare_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                                stmt->right);

  case STMT_VARIABLE_READ:
    return compile_read_variable(com, stmt->sym_decl.name, stmt->sym_decl.type);
  case STMT_VARIABLE_WRITE:
    return compile_write_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                                  stmt->right);

  case STMT_DEF_INVOKE:
    return compile_call_symbol(com, stmt);
  case STMT_CONDITIONAL:
    // printf("\neval conditional:\n");
    // dbg_stmt(stmt);
    return com_conditional(com, stmt->conditional);
  case STMT_DEF_DECL:
    return compile_def_decleration(com, stmt->sym_decl.name, stmt->right);
  case STMT_COMMENT:
    return compile_comment(com, stmt->token);
  }
  // report_syntax_error(stmt->token, "unsupported token");
  Compiler_throw(com, "unsupportd token");
}

void compile_program_with_std(BlockStatement *program, BlockStatement *std,
                              char *filename) {
  Compiler com = Compiler_new();
  compile_block_statement(&com, std);
  compile_block_statement(&com, program);
  Compiler_write_to_file(&com, filename);
}

void compile_program(BlockStatement *program, char *filename) {

  Compiler com = Compiler_new();
  compile_block_statement(&com, program);
  Compiler_write_to_file(&com, filename);
}
