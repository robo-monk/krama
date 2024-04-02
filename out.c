// headers 
#include <stdio.h>
#include <stdlib.h>
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double
#define ptr int*



// declerations 
void println(i32 x);
i32 as_i32(i32 x);
f64 as_f64(f64 x);
i32 fac(i32 a);
i32 exit_success();
i32 tricky(i32 b);
i32 main();


// implementations 
void println(i32 x) {
printf("%d\n", x);;

};
i32 as_i32(i32 x) {
return x;

};
f64 as_f64(f64 x) {
return x;

};
i32 fac(i32 a) {
if (a<2) {
return as_i32(1);
} else {
return a*fac(a-1);
};

};
i32 exit_success() {
return as_i32(1);

};
i32 tricky(i32 b) {
return as_i32(10)+as_i32(b);

};
i32 main() {
i32 bing = tricky(4);
i32 bong = fac(5);
println(bing+bong);
return exit_success();

};

