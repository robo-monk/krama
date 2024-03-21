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
i32 i32memsize();
ptr alloc(i32 size);
void drop(ptr address);
void throw();
void print(i32 x);
ptr array(i32 length);
i32 array_idx(i32 idx);
i32 at(i32 idx, ptr ary);
i32 length(ptr ary);
void set(i32 idx, i32 el, ptr ary);
void initialise(i32 value, i32 start_idx, ptr ary);
void array_print(i32 start, ptr ary);
i32 is_alive(i32 idx, ptr ary);
i32 state_from_neighbours(i32 lcell, i32 ccell, i32 rcell);
i32 rule110(i32 idx, ptr ary);
i32 main();


// implementations 
i32 i32memsize() {
i32 s = (i32) 0;
s = sizeof(i32);;
return s;

};
ptr alloc(i32 size) {
ptr b = (i32) 0;
b = malloc(size);;
return b;

};
void drop(ptr address) {
free(address);;

};
void throw() {
printf("Error");
exit(1);

};
void print(i32 x) {
//#xc printf("[%d]\n", x);
printf("%d", x);

};
ptr array(i32 length) {
ptr pointer = alloc(length*i32memsize()+i32memsize());
pointer[0] = length;
return pointer;

};
i32 array_idx(i32 idx) {
return idx+(i32) 1;

};
i32 at(i32 idx, ptr ary) {
i32 el = (i32) 0;
idx = array_idx(idx);
el = (i32) ary[idx];;
return el;

};
i32 length(ptr ary) {
i32 len = (i32) 0;
len = ary[0];
return len;

};
void set(i32 idx, i32 el, ptr ary) {
if (idx>length(ary)) {
throw();
//#;
};
ary[idx+1] = el;;

};
void initialise(i32 value, i32 start_idx, ptr ary) {
if (start_idx<length(ary)) {
set(start_idx,value,ary);
initialise(value,start_idx+(i32) 1,ary);
//#;
} else {
//#;
};

};
void array_print(i32 start, ptr ary) {
if (start<length(ary)) {
print(at(start,ary));
return array_print(start+(i32) 1,ary);
};

};
i32 is_alive(i32 idx, ptr ary) {
if (idx<(i32) 0) {
return (i32) 0;
} else {
if (idx>length(ary)) {
return (i32) 0;
} else {
return at(idx,ary);
};
};

};
i32 state_from_neighbours(i32 lcell, i32 ccell, i32 rcell){if (lcell&&ccell&&rcell) {return (i32) 0;
}};
i32 rule110(i32 idx, ptr ary) {
i32 lcell = is_alive(idx-(i32) 1,ary);
i32 ccell = is_alive(idx,ary);
i32 rcell = is_alive(idx+(i32) 1,ary);
return state_from_neighbours(lcell,ccell,rcell);

};
i32 main() {
ptr ary = array((i32) 1024*(i32) 10);
//#ary:initialise(1, 0);
set((i32) 0,(i32) 1,ary);
printf("\ndone printing array\n");
array_print((i32) 0,ary);
return (i32) 1;

};

