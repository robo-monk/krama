#include "compiler.h"
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
  for (int i = 0; i < com->headers.size; i++) {
    fprintf(file, "%s\n", (string)vector_at(&com->headers, i));
  }
  // Write the text to the file, followed by a newline character
  fclose(file); // Close the file
}
void compile_program(BlockStatement *program, char *filename) {
  CCompiler com = new_ccompiler();
  write_ccompiler_state_to_file(&com, filename);
}
