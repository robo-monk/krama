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
f64 as_f64(f64 a);
f64 fac(f64 a);


// implementations 
f64 as_f64(f64 a) {
return a;

};
f64 fac(f64 a) {
if (a<(i32) 2) {
return as_f64((i32) 1);
} else {
return a*fac(a-(i32) 1);
};

};

