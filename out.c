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
void throw();
i32 as_i32(i32 x);
i32 memsize(i32 x);
ptr alloc(i32 size);
void drop(ptr address);
void print(i32 x);
void println(i32 x);
ptr array(i32 length);
i32 at(i32 idx, ptr ary);
i32 length(ptr ary);
void set(i32 idx, i32 el, ptr ary);
ptr fill(i32 value, i32 start_idx, ptr ary);
ptr clone(ptr ary);
void main(i32 a);


// implementations 
void throw() {
printf("Error");
exit(1);

};
i32 as_i32(i32 x) {
return x;

};
i32 memsize(i32 x) {
x = sizeof(i32);;
return x;

};
ptr alloc(i32 size) {
ptr x = 0;
x = malloc(size);;
return x;

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
ptr array(i32 length) {
ptr pointer = alloc(length*memsize(length)+memsize(length));
pointer[0] = length;
return pointer;

};
i32 at(i32 idx, ptr ary) {
i32 el = 0;
el = (i32) ary[idx+1];;
return el;

};
i32 length(ptr ary) {
i32 len = 0;
if (ary == NULL) { printf("\nnull pointer exception\n");exit(1); };
len = ary[0];
return len;

};
void set(i32 idx, i32 el, ptr ary) {
if (idx-1>length(ary)) {
throw();
//#;
};
ary[idx+1] = el;;

};
ptr fill(i32 value, i32 start_idx, ptr ary) {
if (start_idx<length(ary)) {
set(start_idx,value,ary);
return fill(value,start_idx+1,ary);
};
return ary;

};
ptr clone(ptr ary) {
ptr cloned = array(length(ary));
i32 len = length(ary);
cloned[0] = len;
for (int i=1; i < len; i++) { cloned[i] = ary[i]; };;
return cloned;

};

void main(i32 a) {
i32 bong = as_i32(a)+as_i32(5);

};

