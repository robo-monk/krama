## TODOs

- [ ] rewrite hashmap myself

Syntax Ideas

```
@enum Type {
  i32, f64
}

@def |a :atom| i32 -> {
  |a|:set_type(Type.i32)
}


@def ops {
  @def |a :f32| -> {
    add(b :f32) -> ...
  }
}

@def |a :f32| {
  @symbol + (b :f32) -> |a|:ops:add(b);
}
```

## Bitboard impl

```css
@type |bits: @mut u64| Bitboard {
    @new -> {
        |bits: 0|:Bitboard
    }
};

@impl |board: Bitboard| {
    mask_bits(mask: u64) -> {
        board:bits:mutate(board:bits & mask)
    }

    set_bit(index: u64) -> {
        @mut mask = 0x000000
        mask_bits(mask)
    }
}

@enum {
    PAWN,
    BISHOP,
    QUEEN,
    KING,
    ROOK,
    KNIGHT
} PieceType;

@type |bb: Bitboard, piece_type: PieceType| PieceBitboard {
    @new(piece_type: PieceType) {
        |:Bitboard, piece_type|:PieceBitboard
    }
}

@impl |bb: PieceBitboard| {
    get_moves() -> {
        @match(bb.piece_type) {
            PAWN -> bb:mask_bits(PAWN_MOVE_MASK)
            PAWN -> bb:mask_bits(PAWN_MOVE_MASK)
        }
    }
}

@let pawn_bb = :PieceBitboard(PAWN);

@let chessboard = |pawn_bb, ...|:Array

chessboard:each
```

## Hello world

```css
"Helloworld":print();
```

## Iterate through an array

```css
@let a = |1, 2, 3, 4|:Array
@let b = [1, 2, 3, 4]

b:for_each(|element, i| -> {
    element:println()
});
```

## Iterate through keys of object

```css
@let o = | keys: |"hello", "there"|:Array, values: |0, 1|:Array |:Hashmap
@let o = {
  "hello":0,"there": 0;
}
o: for_each(|key, value| -> {});
```

## Arrays

```css
@type |...elements| Array {
  @new -> {
    mem: alloc(elements: type: sizeof * elements: count);
  }
  @del -> {
    mem: free();
  }
}

@impl |array: Array| {
}
```

## complex impl

```css
@type |re :f32, im :f32| complex
@def |a :complex| {
  magnitude       -> |a:re*a:re  + a:im*a:im|:sqrt
  add(b :complex) -> |a:re + b:re, a:im + b:im|:complex
  lt(b :complex)  -> a:magnitude > b:magnitude,
}

@def main -> {
    @let a = |4.2, 2.2|:complex
    a:add(a)
}

```

-> desugar

```css
@def |re :f32, im :f32| complex -> {
  @malloc (...);
}

-- std defined stuff
@def |a :complex| {
    to_str   -> f"|re: {a:re}, im: {a:im}| complex"
    clone    -> |a.re, a.im|:complex
}

@def main -> {
    let a = |4.2, 2.2|:complex
    a:add(a)

    @free(a)
}
```

-> C code

```c
float f64_sqrt(float a) {
return ...;
}

typedef struct {
float re;
float im;
} complex;

complex* __complex_new(float re, float im) {
complex* ptr = malloc(sizeof(complex));
ptr->re=re;
ptr->im=im;
return ptr;
}

float complex_magnitude(complex* a) {
return f64_sqrt(a->re*a->re + a->im\*a->im);
}

complex* complex_add(complex* a, complex\* b) {
return __complex_new(a->re + b->re, a->im + b->im);
}
```

@def |a :let| set_type(t :type) {
a.type = t
}

@def |a :i32| factorial -> {
:if | a == 0 | -> 0
:if | a == 1 | -> 1
:else -> a \* :factorial |a-1|
}

let b = |5|:factorial:to_string

```

```

@typ |re :f32, im :f32| complex
@def |a :complex| {
magnitude -> |a:re*a:re + a:im*a:im|:sqrt
add(b :complex) -> |a:re + b:re, a:im + b:im|:complex
lt(b :complex) -> |a:re + b:re, a:im + b:im|:complex
}

let a = |4.2, 2.2|:complex
a:add(a)

```

```

```css
@struct |bits: @mut u64| Bitboard {
    @new -> {
        |bits: 0|:Bitboard
    }
};

@define |board: Bitboard| {
    mask_bits(mask: u64) -> {
        board:bits:mutate(board:bits & mask)
    }

    set_bit(index: u64) -> {
        @mut mask = 0x000000
        mask_bits(mask)
    }
}

@enum {
    PAWN,
    BISHOP,
    QUEEN,
    KING,
    ROOK,
    KNIGHT
} PieceType;

@struct |bb: Bitboard, piece_type: PieceType| PieceBitboard {
    @new(piece_type: PieceType) {
        |:Bitboard, piece_type|:PieceBitboard
    }
}

@define |bb: PieceBitboard| {
    get_moves() -> {
        @match(bb.piece_type) {
            PAWN -> bb:mask_bits(PAWN_MOVE_MASK)
            PAWN -> bb:mask_bits(PAWN_MOVE_MASK)
        }
    }
}

@let pawn_bb = :PieceBitboard(PAWN);

@let chessboard = |pawn_bb, ...|:Array

@define hello_world() -> {
    "hello world" :print
}

```
