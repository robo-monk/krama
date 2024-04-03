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
void print_grid(ptr row);
i32 is_alive(ptr ary, i32 idx);
i32 state_from_neighbours(i32 lcell, i32 ccell, i32 rcell);
i32 rule110(i32 idx, ptr ary);
void apply_rule110(i32 idx, ptr previous, ptr ary);
void iterate_rule110(ptr ary, i32 iterations);
i32 main();


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

void print_grid(ptr row) {
i32 len = length(row);
for (int i = 1; i < len; i++) {if(row[i]){printf("#");}else{printf(".");}};;
printf("\n");

};
i32 is_alive(ptr ary, i32 idx){if (idx<0) {return as_i32(0);
}if (idx>length(ary)-1) {return as_i32(0);
}if (1) {return at(idx,ary);
}};
i32 state_from_neighbours(i32 lcell, i32 ccell, i32 rcell){if (lcell&&ccell&&rcell) {return as_i32(0);
}if (lcell&&ccell&&rcell<1) {return as_i32(1);
}if (lcell&&ccell<1&&rcell) {return as_i32(1);
}if (lcell&&ccell<1&&rcell<1) {return as_i32(0);
}if (lcell<1&&ccell&&rcell) {return as_i32(1);
}if (lcell<1&&ccell&&rcell<1) {return as_i32(1);
}if (lcell<1&&ccell<1&&rcell) {return as_i32(1);
}if (lcell<1&&ccell<1&&rcell<1) {return as_i32(0);
}};
i32 rule110(i32 idx, ptr ary) {
i32 lcell = is_alive(ary,idx-1);
i32 ccell = is_alive(ary,idx);
i32 rcell = is_alive(ary,idx+1);
i32 s = state_from_neighbours(lcell,ccell,rcell);
//##c printf("apply rule to %d%d%d -> %d \n", lcell, ccell, rcell, s);;
return s;

};
void apply_rule110(i32 idx, ptr previous, ptr ary) {
if (idx<length(ary)) {
i32 new_state = rule110(idx,previous);
set(idx,new_state,ary);
return apply_rule110(idx+as_i32(1),previous,ary);
};
//#;

};
void iterate_rule110(ptr ary, i32 iterations) {
if (iterations>0) {
ptr cloned = clone(ary);
apply_rule110(0,cloned,ary);
drop(cloned);
print_grid(ary);
return iterate_rule110(ary,iterations-1);
};
//#;

};
i32 main() {
ptr row = fill(0,0,array(31));
set(29,1,row);
print_grid(row);
iterate_rule110(row,42);
return as_i32(0);

};

