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
ptr aloc(i32 size);


// implementations 
ptr aloc(i32 size) {
ptr adr = 0;
adr = malloc(adr);
return adr;

};

