#include "compiler.h"
#include "../frontend/LiteralType.h"
#include "../hashmap/hashmap.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include <complex.h>

void throw_compilation_error(const char *format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, "\033[31m\n[compiler] Error! "); // Start red text
  vfprintf(stderr, format, args); // Print formatted output in red
  fprintf(stderr, "\033[0m\n");   // Reset to default text color

  va_end(args);
  exit(1);
}

int __sym_compare(const void *a, const void *b, void *udata) {
  const Symbol *ua = a;
  const Symbol *ub = b;
  return strcmp(ua->name, ub->name);
}

uint64_t __sym_hash(const void *item, uint64_t seed0, uint64_t seed1) {
  const Symbol *var = item;
  return hashmap_sip(var->name, strlen(var->name), seed0, seed1);
}

Symbol *new_sym(string name) {
  Symbol *sym = malloc(sizeof(Symbol));
  sym->name = name;
  return sym;
}

Symbol *new_def_symbol(string name, Statement *body) {
  Symbol *sym = new_sym(name);
  DefSymbol *def = malloc(sizeof(DefSymbol));
  def->body = body;

  // TODO: infer type
  def->type = LiteralType_void;
  sym->def = def;
  return sym;
}

Symbol *new_var_symbol(string name, LiteralType type) {
  Symbol *sym = new_sym(name);
  sym->var = malloc(sizeof(VariableSymbol));
  sym->var->type = type;
  return sym;
}

Symbol const *com_get_symbol(CCompiler *com, string sym_name) {
  return hashmap_get(com->sym_map, &(Symbol){.name = sym_name});
}

VariableSymbol *com_get_var_sym(CCompiler *com, string var_name) {
  const Symbol *sym = com_get_symbol(com, var_name);
  if (sym == NULL) {
    return NULL;
  }
  return sym->var;
}

DefSymbol *com_get_def_sym(CCompiler *com, string var_name) {
  const Symbol *sym = com_get_symbol(com, var_name);
  if (sym == NULL) {
    return NULL;
  }
  return sym->def;
}

void com_declare_def_sym(CCompiler *com, string def_name, Statement *body,
                         LiteralType return_type) {

  printf("   DEFINE %s w/ return type %s \n", def_name,
         literal_type_to_str(return_type));

  if (com_get_def_sym(com, def_name) != NULL) {
    throw_compilation_error("redecleration of definition '%s'", def_name);
  }

  hashmap_set(com->sym_map, new_def_symbol(def_name, body));
}

void com_declare_var_sym(CCompiler *com, string var_name, LiteralType type) {
  printf("   DECLARE %s of type %s \n", var_name, literal_type_to_str(type));
  if (com_get_var_sym(com, var_name) != NULL) {
    throw_compilation_error("redecleration of variable '%s'", var_name);
  }
  hashmap_set(com->sym_map, new_var_symbol(var_name, type));
}

CCompiler new_ccompiler() {
  CCompiler com = {.headers = new_vec(1024, sizeof(string)),
                   .implementations = new_vec(1024, sizeof(string))};

  com.sym_map = hashmap_new(sizeof(SymbolStatement), 0, 0, 0, __sym_hash,
                            __sym_compare, NULL, NULL);

  str_vector_push(&com.headers, "#define i32 int\n"
                                "#define i64 long\n"
                                "#define u32 unsigned int\n"
                                "#define u64 unsigned long\n"
                                "#define f32 float\n"
                                "#define f64 double\n");
  return com;
}

void push_implementation(CCompiler *com, string impl_str) {
  str_vector_push(&com->implementations, impl_str);
}
void push_header(CCompiler *com, string head_str) {
  str_vector_push(&com->implementations, head_str);
}

// StrVec com_block_statement(CCompiler *com, BlockStatement *stmt);
string com_block_statement(CCompiler *com, BlockStatement *stmt) {
  StrVec statements = new_str_vec(512);
  for (int i = 0; i < stmt->len; i++) {
    printf("\n COM BLOCK ");
    str_vector_push(&statements, com_statement(com, stmt->statements[i]));
    str_vector_push(&statements, ";\n");
  }

  string a = str_vector_join(&statements);
  printf("\n\nvector join is %s\n\n", a);
  push_implementation(com, a);
  return a;
}

string com_write_variable(CCompiler *com, string var_name, LiteralType var_type,
                          string value) {

  VariableSymbol *varsym = com_get_var_sym(com, var_name);
  if (varsym == NULL) {
    throw_compilation_error("tried to write an undeclared variable '%s'",
                            var_name);
  }
  // if (com_get_symbol(com, var_name)) {
  // }
  return concat(3, var_name, " = ", value);
}

string com_declare_variable(CCompiler *com, string var_name,
                            LiteralType var_type, string value) {
  com_declare_var_sym(com, var_name, var_type);

  return concat(3, literal_type_to_str(var_type), " ",
                com_write_variable(com, var_name, var_type, value));
}

string com_read_variable(CCompiler *com, string var_name,
                         LiteralType var_type) {
  VariableSymbol *varsym = com_get_var_sym(com, var_name);
  if (varsym == NULL) {
    throw_compilation_error("tried to read from undeclared variable '%s'",
                            var_name);
  }
  return concat(4, "(", literal_type_to_str(var_type), ")", var_name);
}

string com_bin_op(string left, string right, OpType optype) {
  return concat(3, left, optype_to_str(optype), right);
}

string com_conditional(CCompiler *com, ConditionalStatement *conditional) {
  string else_com = "";
  if (conditional->else_body) {
    else_com = concat(3, " else {\n",
                      com_block_statement(com, conditional->else_body), "}");
  }

  return concat(6, "if (", com_statement(com, conditional->condition), ") {\n",
                com_block_statement(com, conditional->if_body), "}", else_com);
}

string compile_block_arguments(CCompiler *com, BlockStatement *block) {
  // string compiled_args = malloc(sizeof(char) * 512);
  StrVec compiled_args = new_str_vec(1);
  for (int i = 0; i < block->arg_len; i++) {
    if (i != 0) {
      str_vector_push(&compiled_args, ", ");
    }

    string arg_str = concat(3, literal_type_to_str(block->args[i]->type), " ",
                            block->args[i]->name);

    str_vector_push(&compiled_args, arg_str);
  }

  return str_vector_join(&compiled_args);
}

string com_def_declaration(CCompiler *com, string def_name, Statement *stmt) {
  if (com_get_def_sym(com, def_name)) {
    throw_compilation_error("redecleration of definition'%s'", def_name);
  }
  LiteralType return_type = LiteralType_void;
  com_declare_def_sym(com, def_name, stmt, return_type);
  // body->block->
  // args parsing
  string compiled_args = compile_block_arguments(com, stmt->block);

  printf("\n -- compiled args %s\n", compiled_args);

  return concat(6, literal_type_to_str(return_type), " ", def_name, "(",
                compiled_args, ")");

  // new_def_symbol(char *name, BlockStatement *body)
  // com_declare_def_sym(com, stmt->sym_decl.name, stmt->, LiteralType
  // return_type)
  // string else_com = "";
  // if (conditional->else_body) {
  //   else_com = concat(3, " else {\n",
  //                     com_block_statement(com, conditional->else_body), "}");
  // }

  // return concat(6, "if (", com_statement(com, conditional->condition), ")
  // {\n",
  //               com_block_statement(com, conditional->if_body), "}",
  //               else_com);
}

string com_statement(CCompiler *com, Statement *stmt) {
  switch (stmt->type) {
  case STMT_BLOCK:
    return com_block_statement(com, stmt->block);
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
    return com_read_variable(com, stmt->sym_decl.name, stmt->sym_decl.type);
  case STMT_VARIABLE_WRITE:
    return com_write_variable(com, stmt->sym_decl.name, stmt->sym_decl.type,
                              com_statement(com, stmt->right));

  // case STMT_DEF_INVOKE:
  //   return com_call_symbol(com, stmt);
  case STMT_CONDITIONAL:
    // printf("\neval conditional:\n");
    // dbg_stmt(stmt);
    return com_conditional(com, stmt->conditional);
  case STMT_DEF_DECL:
    return com_def_declaration(com, stmt->sym_decl.name, stmt->right);
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
    printf("\n\n %d*** %s\n", i, (string)vector_at(&com->implementations, i));
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
