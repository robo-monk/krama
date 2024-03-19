// headers 
#define i32 int
#define i64 long
#define u32 unsigned int
#define u64 unsigned long
#define f32 float
#define f64 double



// declerations 
i32 fib(i32 a);
f64 as_float(f64 a);
i32 as_int(i32 a);
f64 main();


// implementations 
i32 fib(i32 a) {
return if ((i32)a<(i32)2) {
return (i32)1;
} else {
return fib((i32)a-(i32)1)+fib((i32)a-(i32)2);
};

};
f64 as_float(f64 a) {
return (i32)a;

};
i32 as_int(i32 a) {
return (i32)a;

};
f64 main() {
return if ((i32)1) {
return as_float((i32)5);
} else {
return as_int((i32)5);
};

};

