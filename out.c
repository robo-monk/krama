// headers 
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double



// declerations 
i32 add(i32 a, i32 x);
i32 sub(i32 a, i32 x);
i32 mul(i32 a, i32 x);
i32 to_f64(i32 x);
i32 ppow(i32 a, i32 x);
i32 fac(i32 x);
i32 hello();
i32 int_main();


// implementations 
i32 add(i32 a, i32 x) {
return x+a;

};
i32 sub(i32 a, i32 x) {
return x-a;

};
i32 mul(i32 a, i32 x) {
return x*a;

};
i32 to_f64(i32 x) {
return x;

};
i32 ppow(i32 a, i32 x) {
if (a>(i32) 0) {
return mul(ppow(sub((i32) 1,a),x),x);
} else {
return (i32) 1;
};

};
i32 fac(i32 x) {
if (x<(i32) 2) {
return (i32) 1;
} else {
return x*fac(x-(i32) 1);
};

};
i32 hello() {
i32 a = (i32) 4;
i32 b = fac((i32) 2);
return fac(ppow(b,a));

};
i32 int_main() {
return hello();

};

#include "stdio.h"
int main() { printf("%d", int_main()); return 1;}
