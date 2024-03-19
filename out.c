// headers 
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double



// declerations 
f64 recursive(f64 a);
i32 add(i32 a, i32 b);
i32 sub(i32 a);
i32 main();


// implementations 
f64 recursive(f64 a) {
if (a>(i32) 0) {
return recursive(a-(i32) 1);
} else {
return a;
};

};
i32 add(i32 a, i32 b) {
return a;

};
i32 sub(i32 a) {
return a+a;

};
i32 main() {
if ((i32) 1) {
return sub((i32) 390)+add((i32) 4892,(i32) 3);
} else {
return add((i32) 390,(i32) 39)+add((i32) 4892,(i32) 3);
};

};

