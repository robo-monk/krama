#include "../frontend/LiteralType.h"
#include "../hashmap/hashmap.h"
#include "compiler.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include <complex.h>

void Compiler_throw(const char *format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, "\033[31m\n[Compiler] Error! "); // Start red text
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
                             LiteralType return_type) {

  printf("\nDECLARE DEFINITION %s w/ return type %s \n", def_name,
         literal_type_to_str(return_type));

  if (Compiler_defsym_get(com, def_name) != NULL) {
    Compiler_throw("redecleration of definition '%s'", def_name);
  }

  hashmap_set(com->sym_map, new_def_symbol(def_name, body));
}

void Compiler_varsym_declare(Compiler *com, string var_name, LiteralType type) {
  printf("\nDECLARE %s of type %s \n", var_name, literal_type_to_str(type));
  if (Compiler_varsym_get(com, var_name) != NULL) {
    Compiler_throw("redecleration of variable '%s'", var_name);
  }
  hashmap_set(com->sym_map, new_var_symbol(var_name, type));
}

Compiler Compiler_new() {
  Compiler com = {.headers = new_str_vec(1),
                  .implementations = new_str_vec(1),
                  .declerations = new_str_vec(1),
                  .upper = NULL,
                  .sym_map = malloc(sizeof(Scope))};

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
  // Write the text to the file, followed by a newline character
  fclose(file); // Close the file
}