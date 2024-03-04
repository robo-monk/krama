#include "utils.h"
#include "stdio.h"
#include "stdlib.h"

bool is_digit(char c) { return c >= '0' && c <= '9'; }

int parse_digit(char c) {
  if (is_digit(c)) {
    return c - '0';
  }
  printf("%c is not a number", c);
  exit(1);
}

int str2int(string str, int len) {
  int n = 0;
  for (int i = 0; i < len; i++) {
    n = n * 10 + parse_digit(str[i]);
  }
  return n;
}
