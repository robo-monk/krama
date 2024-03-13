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

```
shape Array {
  length: u32
  type: any
  _ptr -> c:malloc(length * type:size)
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
