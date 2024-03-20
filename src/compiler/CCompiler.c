#include "CCompiler.h"
#include "../frontend/LiteralType.h"
#include "../frontend/parser.h"
#include "../hashmap/hashmap.h"
#include "inference.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"

void Compiler_throw(Compiler *com, const char *fmt, ...) {
  char buffer[1024];   // Buffer to hold the formatted error message
  va_list args;        // To handle the variable argument list
  va_start(args, fmt); // Initialize the variable arguments list
  vsnprintf(buffer, sizeof(buffer), fmt, args); // Create formatted string
  va_end(args); // Clean up the variable arguments list

  report_syntax_error(com->current_stmt->token, buffer);
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

Symbol *new_def_symbol(string name, Statement *body,
                       BranchLiteral *return_type) {
  Symbol *sym = new_sym(name);
  DefSymbol *def = malloc(sizeof(DefSymbol));

  def->body = body;
  def->btype = return_type;
  sym->def = def;
  return sym;
}

Symbol *new_var_symbol(string name, LiteralType type) {
  Symbol *sym = new_sym(name);
  sym->var = malloc(sizeof(VariableSymbol));
  sym->var->type = type;
  return sym;
}

Symbol const *Compiler_symbol_get(Compiler *com, string sym_name) {
  return hashmap_get(com->sym_map, &(Symbol){.name = sym_name});
}

VariableSymbol *Compiler_varsym_get(Compiler *com, string var_name) {
  const Symbol *sym = Compiler_symbol_get(com, var_name);
  if (sym == NULL) {
    return NULL;
  }
  return sym->var;
}

DefSymbol *Compiler_defsym_get(Compiler *com, string def_name) {
  const Symbol *sym = Compiler_symbol_get(com, def_name);
  if (sym == NULL) {
    // bubble up scope
    if (com->upper != NULL)
      return Compiler_defsym_get(com->upper, def_name);

    return NULL;
  }
  return sym->def;
}

void Compiler_defsym_declare(Compiler *com, string def_name, Statement *body,
                             BranchLiteral *return_type) {

  printf("\nDECLARE DEFINITION %s w/ return type", def_name);
  dbg_branch_literal(return_type);

  if (Compiler_defsym_get(com, def_name) != NULL) {
    Compiler_throw(com, "redecleration of definition '%s'", def_name);
    // Compiler_throw("redecleration of definition '%s'", def_name);
  }

  hashmap_set(com->sym_map, new_def_symbol(def_name, body, return_type));
}

void Compiler_varsym_declare(Compiler *com, string var_name, LiteralType type) {
  printf("\nDECLARE %s of type %s \n", var_name, literal_type_to_str(type));
  if (Compiler_varsym_get(com, var_name) != NULL) {
    Compiler_throw(com, "redecleration of variable '%s'", var_name);
  }
  hashmap_set(com->sym_map, new_var_symbol(var_name, type));
}

Compiler Compiler_new() {
  Compiler com = {.headers = new_str_vec(1),
                  .implementations = new_str_vec(1),
                  .declerations = new_str_vec(1),
                  .upper = NULL,
                  .sym_map = malloc(sizeof(Scope)),
                  .current_stmt = NULL};

  com.sym_map = hashmap_new(sizeof(SymbolStatement), 0, 0, 0, __sym_hash,
                            __sym_compare, NULL, NULL);

  str_vector_push(&com.headers, "#define i32 int\n"
                                "#define i64 long\n"
                                "#define u32 unsigned int\n"
                                "#define u64 unsigned long\n"
                                "#define f32 float\n"
                                "#define f64 double\n"
                                "#include <stdio.h>");
  return com;
}

void Compiler_free(Compiler *com) {
  Vec_free(&com->declerations);
  Vec_free(&com->implementations);
  Vec_free(&com->headers);
  hashmap_free(com->sym_map);
}

void Compiler_impl_push(Compiler *com, string impl_str) {
  str_vector_push(&com->implementations, impl_str);
}
void Compiler_decl_push(Compiler *com, string decl_str) {
  str_vector_push(&com->declerations, concat(2, decl_str, ";"));
}
void Compiler_header_push(Compiler *com, string head_str) {
  str_vector_push(&com->implementations, head_str);
}

void Compiler_write_to_file(Compiler *com, string filename) {
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

  fprintf(file, "\n\n// declerations \n");
  for (int i = 0; i < com->declerations.size; i++) {
    // printf("\n\n %d*** %s\n", i, (string)vector_at(&com->declerations, i));
    fprintf(file, "%s\n", (string)vector_at(&com->declerations, i));
  }

  fprintf(file, "\n\n// implementations \n");
  for (int i = 0; i < com->implementations.size; i++) {
    // printf("\n\n %d*** %s\n", i, (string)vector_at(&com->implementations,
    // i));
    fprintf(file, "%s\n", (string)vector_at(&com->implementations, i));
  }

  // fprintf(file, "%s\n",
  //         "#include \"stdio.h\"\nint main() { printf(\"%d\", int_main()); "
  //         "return 1;}");

  fclose(file); // Close the file
}
