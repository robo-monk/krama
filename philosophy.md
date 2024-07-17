# Philosophy

### Occam's razor

In philosophy, Occam's razor is the problem-solving principle that recommends searching for explanations constructed with the smallest possible set of elements.

### Krama's philosophy

Krama is aiming to solve 3 main problems with current programming langauges.

1. bloat
2. ugly
3. overhead

Krama should offer the same low-level memory management as C but look as simple as python. This will require strong, meaningful default behaviour but also the option to customise the behaviour deeply for specific usecases

### As simple as possible

Parameters of simplicity in a programming langauge.

1. Amount of keywords and symbols someone needs to remember
2. Delta from current state of most languages
3. Magic going on "under the hood"

Rust for example, allthough quite simple, has an extreme learning curve. You end up using "built-in" things from std lib that doing extremely complicated things under the hood. Borrowing all the time makes the language verbose and ugly. It's beautiful in the sense that once you win the fight over the compiler your code is usually sound, however the fight to get there requires bloodpath with lifetimes, amperstands, verbose trait constraints and stuff you dont really care and just want to work by themsevles.

Python on the other hand has hacks within the language to make basic features work, like enums. There also pythonic ways to do things, but also the language allows "non pythonic" ways to do things. Why is somehting un-pythonic allowed?

Krama will attempt to be extremely simple, and will take the path of simplicity at all times even if itmeans that certain tasks are cumbersome/impossible to achieve

### About Generics

The problem that Generics<T> aim to solve, is to be able to write abstract code -- code that is valid and can be executed for structures with certain properties. Thus, these structures can be different from each other, but share the generic code.

The way you can define which properties you need is through generics. The idea is beautiful, however it introduces a lot of complexity to the language. Thus Krama, does not have generics in the same sense as Rust, Zig or C++.

Instead, 

```
sig k = |x: i32|

mut x = 39032.0

for i in 1..0 {
  print(i)
}

fn bingbong(x: hello) {
  if (x == "hello") -> x
  -> x
}


### Ideas
* array indexes are types
* use @ to declare compile time operations, make it clear its compile time
    - all @ all actually macros
* defer keyword to run at the end of scope
* reactive features like signals?
```
