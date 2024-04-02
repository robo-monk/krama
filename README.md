# ㊎ Krama

A toy programming language bootstrapped in C.

## TODOs

- [ ] inference to variable declerations
- [ ] can't assign f64 to i32 symbol
- [ ] `Vec` is terrible, it is a vector of pointers, there's no point into initialising it with size as all elements are pointers

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

## Procedures for reciever

Procedures are linear processes that are guaranteed to have same output for same input. They do not possess access to higher scope. They get lazy evaluated when they are needed. They can have a receiver or be defined to the module's scope.

```
proc |x: u64| add(a: u64) -> x + a

let a = 15
a~add(4)        # throws "a is not mutable"
print(a:add(4)) # prints 19, a remains 15

mut b = 20
b~add(4)
print(b) # 24
```

#### Shorthand for Multiple Procedures for reciever

```
proc |x: u64| {
  add(a: u64) -> x + a
  sub(a: u64) -> x - a
  mul(a: u64) -> x * a
}
```

#### Shapes

Shapes are also linear (they're evaluated from top to bottom), they can have arguments and they are the **only** way that you can allocate memory in krama.

They belong to the `proc` that spawns them and get deallocated once the procedure completes.

```
shape List(type: Type) {

}
proc |list: List| {
  append(thing: of(list.type))
}

shape TokeniserBuffer {
  mut str: string -> "",
  mut type: BASE10_NUM | BASE2_NUM | BASE16_NUM | FLOAT | STRING
}

impl |buffer: TokeniserBuffer| {

  tree set_type(c: char) {
    buffer:is_empty -> buffer.type~mutate(chartype(c)),
    buffer.type == BASE10_NUM -> {

    }
  }

}

shape Tokeniser(content: string) {
  content: string -> content,
  mut escape: bool -> false,
  mut tokens -> List(Token)
  mut buffer -> CharList
}


proc |t: Tokeniser| {
  tokenise_char(c: char) {
    match {
      '/r' | '/n' -> {
        t~commit_buffer()
        t.escape = false
      },
      '#' -> {
        t.escape = true
        t~push_to_buffer(c)
      }
    }
  }

  append_token(token: Token) {
    t.tokens~append(token)
  }

  push_to_buffer(own c: char) {
    tree {
      (t.buffer:len() == 0) -> t.buffer.type = detect_chartype(c),
      (t.buffer.type == BASE10_NUM)
    }

    t.buffer~append(c)
  }

  commit_buffer() -> {
    t.tokens~append(token)
  }

  close() -> {
    t:commit_buffer()
    t:push_token(Token.PROGRAM_END)
  }
}
```

## Trees, Procedures and Shapes

```

# nothing from outer scope can get in
# all parameters are immutable
# all memory allocated within gets destroyed at the end
proc foo(a: i32, b: string) -> {
}


# nothing from outer scope can get in
# all parameters are immutable
# all memory allocated within gets destroyed at the end
proc |x: i32| add(a: i32) -> x + a

# Bitboard64.kr
macro HotBitmask   (i: Bitboard64Index) ->   u64(0) << i
macro NotHotBitmask(i: Bitboard64Index) -> !(u64(0) << i)

shape Bitboard64 {
  bits -> u64(0)
}

impl |b: Bitboard64| {
  proc mask(mask: u64)                 -> b.bits:bin_and(mask)
  proc get_bit(idx:   Bitboard64Index) -> b:mask(HotBitmask(idx))
  proc set_bit(idx:   Bitboard64Index) -> b.bits~bin_or(HotBitmask(idx))
  proc unset_bit(idx: Bitboard64Index) -> b.bits~bin_and(NotHotBitmask(idx))
}

shape Piece(pieceType: PieceType, color: Color) {
  pieceType, color
}

# Board.kr
shape Board(color: Color) {
  mut turn: Color = color
  white -> Array(Bitboard64, 7)
  black -> Array(Bitboard64, 7)
  mut en_passant_target -> Option(usize)
  white_cr -> CastlingRights()
  black_cr -> CastlingRights()
}

target |bb: Bitboard| {
  unplace(piece: Piece, index: u64) {
  }
}

target |bb: Bitboard| {
  mutation place(piece: Piece, index: u64) {
    bb
  }

  decision

  proc for_piece(piece: Piece) {
    let index = piece:bb_index()
    match |piece.color| {
      Color.White -> bb.white:at(index)
      Color.Black -> bb.black:at(index)
    }
  }

  proc place(piece: Piece, index: u64) {
    bb:for_piece(piece)~set_bit(index)
  }

  mutation unplace(piece: Piece, index: u64) {
    bb:for_piece(piece):mutate:unset_bit(index)
  }

  proc load_fen(fen: string) {
    # ...
    let piece = Piece(PieceType.PAWN, Color.White)
    bb.place(piece, 32)
  }
}

proc main() {
  let bitboard = Bitboard(Color.White)
}
```

```

shape Bitboard {
  mut bits: u64
}

target |b: Bitboard| {
  mutation set_bit(index: u64) -> b.bits = bits & one_hot_mask(index)
  function get_bit(index: u64) -> b.bits & one_hot_mask(index)
}

target |b: Bitboard64| {
  proc mask(mask:     u64)             -> b.bits:bin_and(mask)

  proc get_bit(idx:   Bitboard64Index) -> b:mask(HotBitmask(idx))

  proc set_bit(idx:   Bitboard64Index) -> b.bits~bin_or(HotBitmask(idx))

  proc unset_bit(idx: Bitboard64Index) -> b.bits~bin_and(NotHotBitmask(idx))
}

shape List<T>(initial_len: u64) {
  address: ptr  -> calloc(T.size, initial_len)
  mut idx: u64  -> 0
  max_len: u64  -> initial_len
}

# targets an element named `a` which is a list
target |a: List<T>| {

  # prefix a mutation with ~ to invoke it
  # a~append()
  # the own keyword transfers the ownership to the mutation's scope
  pub mt append(own el: T) -> {
    a.address[a.idx] = el
    a.idx += 1
  }

  pub mt pop() -> {
    a.idx -= 1
    a.address[a.idx+1]
  }

  pub fn for_each(cb: Fn()) ->
    a:len:times:do(|i| -> cb(a:at(i), i))

  # prefix a function with : to invoke it
  # let length = a:len()
  pub function len() -> a.idx + 1
}

```

Keywords Reserved

```

`mt` -> defines opening for a mutation definition
`fn` -> defines opening for a function definition
`pub` -> used for defining scope for functions, mutations and variablees

`own` -> can be used as prefix of an argument name to transfer ownership to the mutation
```

Symbols

```
":" -> after a argument name / variable name to define the type of it
    -> before a function call to invoke it

"~" -> invoke a mutation
"->" -> return
```

[ ] assumption: only mutations will ever need to "own" something

````

#### Trees

Map an input `X` to an output `F(X)`, based on conditions `G(X)`

```krama
tree state_from_neighbours (lcell: i32, ccell: i32, rcell: i32) {
    (lcell && ccell && rcell)    ->  0   // 111 -> 0
    (lcell && ccell && !rcell)   ->  1   // 110 -> 1
    (lcell && !ccell && rcell)   ->  1   // 101 -> 1
    (lcell && !ccell && !rcell)  ->  0   // 100 -> 0
    (!lcell && ccell && rcell)   ->  1   // 011 -> 1
    (!lcell && ccell && !rcell)  ->  1   // 010 -> 1
    (!lcell && !ccell && rcell)  ->  1   // 001 -> 1
    (!lcell && !ccell && !rcell) ->  0   // 000 -> 0
}
````

```krama
shape Array(el_type: Type) {
  len: u64
  addr -> alloc(len * el_type:size)
}

def main() {
  let ary = Array(u64):()
  ary:set(0, 5);
}
```

```krama
shape Token {
}
```

### Loops

#### Procedures

Apply procedure `F` on input `x` and get output `F(x)`

```krama

```

### Shapes

````

## Hello World

```krama
:println("Hello World")
````

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

:hello("World")
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
