// headers 
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double
#include <stdio.h>


// declerations 
void print(i32 x);
i32 main();


// implementations 
void print(i32 x) {
printf("%d", x);

};
i32 main() {
i32 c = (i32) 550;
print(c);
return c;

};

