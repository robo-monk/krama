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
i32 mul(i32 a, i32 x);
i32 as_i32(i32 x);
i32 alloc(i32 size);
void drop(ptr address);
void print(i32 x);
void println(i32 x);
i32 array(i32 size);
i32 at(i32 idx, ptr ary);
void main();


// implementations 
i32 mul(i32 a, i32 x) {
return x*a;

};
i32 as_i32(i32 x) {
return x;

};
i32 alloc(i32 size) {
ptr x = 0;
x = malloc(size);;
return as_i32(x);

};
void drop(ptr address) {
free(address);;

};
void print(i32 x) {
printf("%d", x);

};
void println(i32 x) {
printf("%d\n", x);

};
i32 array(i32 size) {
return alloc(size);

};
i32 at(i32 idx, ptr ary) {
i32 el = 0;
el = ary[idx];;
return el;

};

void main() {
return println(420);

};

