#include "stdio.h"
// headers
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double

// declerations
void main();

// implementations
i32 fib(i32 a) {
  if ((i32)a < (i32)2) {
    return (i32)1;
  } else {
    return fib((i32)a - (i32)1) + fib((i32)a - (i32)2);
  };
};
void main() {
  i32 b = fib((i32)39);
  printf("fib: %d", b);
};
