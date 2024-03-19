// headers
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double

// declerations
i32 fib(i32 fib_arg);
i32 main();

// implementations
i32 fib(i32 fib_arg) { return (i32)fib_arg * (i32)fib_arg; };
i32 main() {
  i32 b = (i32)42;
  i32 c = (i32)300;
  return fib((i32)b + (i32)c);
};
