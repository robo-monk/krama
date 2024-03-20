// headers 
#include <stdio.h>
#include <stdlib.h>
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double
#define ptr void*



// declerations 
i32 alloc(i32 size);
void drop(ptr address);
void print(i32 x);
i32 array(i32 size);
i32 at(i32 idx, ptr ary);
i32 main();


// implementations 
//# #include <stdio.h>;
//# #include <stdlib.h>;
i32 alloc(i32 size) {
ptr b = (i32) 0;
b = malloc(size);;
f64 c = (i32) 0;
return c;

};
void drop(ptr address) {
free(address);;

};
void print(i32 x) {
printf("%d", x);

};
i32 array(i32 size) {
return alloc(size);

};
i32 at(i32 idx, ptr ary) {
i32 el = (i32) 0;
el = ary[idx];;
return el;

};
i32 main() {
i32 c = (i32) 550;
print(c);
ptr ary = array((i32) 10);

};

