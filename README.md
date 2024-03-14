# ㊎ Krama

A toy programming language bootstrapped in C.

## Roadmap

- [x] Expression Parsing (2 \* 2 + 4)
- [x] Functions
- [x] Conditional Statements
- [x] Turing Complete
- [ ] Solid Syntax & Grammar Definition
- [ ] Implement bool type
- [ ] Implement char type
- [ ] Arrays
- [ ] Strings
- [ ] Krama -> C Compilation
- [ ] Krama to be written in Kram

## Hello World

```krama
:println("Hello World")
```

## Variable Declaration

```krama
let a: u32 = 5    // immutability by default
mut b: i32 = 64   // explicit mutability
```

## Function Declaration

```krama
def hello(suffix: string) -> {
  :print("Hello", suffix)
}

:hello("World)
```

Symbols declared using the `def` syntax, are available to be invoked by all scopes.

```krama
let bing = "bing"
def bong() -> {
  bing // does not work
}
```

## Function Declaration for objects

```krama
// x can be any shape with a method named mul
def |x: with(:mul)| factorial -> {
  if (x == 0) return 0
  x * (x-1):factorial
}

5:factorial // -> 120
```

```krama
def print(...args: with(:to_string)[] ) -> {
  if (args.length == 0) return C:puts(arg)
  args:each(|arg| -> C:puts(arg + " "))
}

def println(...args: with(:to_string)[] ) -> {
  :print(...args)
  :print("\n")
}
```

## Arrays

```krama
let foo = [32, 543, 113]
print(foo) // [32, 543, 113]

let size = 5
let food: [i32; 4] = []
```

## Vectors

```
shape Vec {
  capacity: u64
}
```

```krama
let bar = Vec:new { capacity: 5 }

with_capacity(capacity: u32) -> {
  Vec:new({capacity})
}
```

## Shapes

```krama
// lib/email.kr
shape Email {
  str: string
  is_valid -> {
    EMAIL_REGEX.test(str)
  }
}

def |str: string| as_email -> {
  let email = Email { str }
  if !email.is_valid {
    raise "bad email"
  }
  email
}

def |email: Email| {
  extract_provider -> {
    email.str:split('@')[1]
  }

  extract_handle -> {
    email.str:split('@')[0]
  }

  is_fraudulant -> {
    FRAUDULANT_PROVIDERS:contains(email:extract_provider)
  }
}

// lib/user.kr
include "lib/email.kr"
shape User {
  active: bool,
  username: string,
  email: Email,
  is_fraudulant -> email:is_fraudulant
}

// src/main.kr
import { User } from "lib/user.kr"
import "lib/email.kr"

let user = User {
  active:     true,
  username:   "hello",
  email:      "bingbong@gmail.com":as_email
}
```

### Internals

````krama
shape Vec {
  capacity: u64
}

let vec = Vec:new({ capacity: 5 })

The above definition will get expanded to
```krama
#define __Vec_SIZE = 64 // <-- evaluated by the compiler
def Vec {
  new({ capacity: u64 }) -> {
    __malloc(__Vec_SIZE) // returns pointer to vec
  }

  destroy(instance: Vec) -> {
    __free(instance)
  }
}

let vec: Vec* = Vec:new({capacity:5})

````

```c
// and then to C
struct Vec {
  unsigned long capacity
}

*Vec Vec_new(long capacity) {
  return malloc(sizeof(Vec))
}

void Vec_destroy(*Vec instance) {
  return free(instance)
}

const Vec* vec = Vec_new(5)
```

```krama
// x can be any shape with a method named mul
def |x: u32| factorial -> {
  if (x == 0) return 0:u32
  x * (x-1):factorial
}

5:factorial // -> 120
```

```
u32 u32_factorial(x: u32) {
  if (x == 0) return (u32) 0;
  return x * factorial(x-1);
}
factorial(5);
```

### STD_LIB

```krama
// default definitions for shapes
def |x: Shape| {
  to_string -> {
  }
}
```

shape Array {
length: u32
type: any
\_ptr -> c:malloc(length \* type:size)
}

forge |array: Array| {
each(cb: Closure, start=0, end=array.length) -> {
}
}

```

## Syntax Primitives

#### Closures

|<Identifier>: <Type>, ...| -> { ...scoped execution }

|<Identifier>: <Type>, ...| {
<ClosureName> -> { ...scoped execution }
}
```
