# Krama Language Specification

`art` -> defines an immutable reference to an artefact
`mut` -> defines a mutable variable
`let` -> defines a immutable variable
`<-` -> binds and assigns a value to an artefact
`->` -> is a "lazy" opearator. the rhs will get evaluated or returned only when particular circumstances are met depending on the context

`shape` -> is used to define custom data structures
`target` -> is used to define implementation details for a specific target. The <pattern> can match various variables and artifacts
`mt` -> defines a mutator. defines a procedure that can mutate the matched target within a target block
`fn` -> defines a function. defines a procedure that can read the state of the target its defined in and return a value

Shape Definition:

`shape` is used to define custom data structures or "shapes" in Krama, akin to classes or structs in other programming languages. It allows for the encapsulation of data and behavior within a single, user-defined type.

Shapes can have properties (data members) and methods (functions or mutators) defined within them, allowing for both data storage and behavior implementation. Properties can be mutable (mut) or immutable (let), and methods are defined as either functions (fn) that do not alter the shape's state or mutators (mt) that do.

Target Definition and Usage:

The target keyword introduces a scope for defining functions (fn) and mutators (mt) that operate on a specific instance of a shape or data structure. It is a way to define behavior that is closely associated with a particular data type.
Within a target block, one can define how instances of the shape interact with the functions or mutators, essentially specifying the operations that can be performed on or with the instances of the defined shape.
This concept emphasizes the encapsulation of behavior with data, allowing for a clear and structured approach to object-oriented programming within Krama. It ensures that functions and mutators are explicitly bound to the data structures they operate on, enhancing code readability and maintainability.

Ownership and Memory Management:

Variables are automatically dropped at the end of their scope, unless their ownership is transferred.
The own keyword indicates that a function or mutator takes ownership of a variable, moving it into the new scope. This prevents the variable from being dropped in the original scope and ensures it's dropped in the new scope unless its lifetime is explicitly extended.
To extend a variable's lifetime beyond its current scope, its lifetime can be bound to an artefact using the <- operator.
Mutability and Immutability:

Immutable variables are declared using let, and mutable variables are declared using mut.
Functions (fn) do not mutate their target and are invoked with the syntax target:foo(args).
Mutators (mt) can mutate their target and are invoked with the syntax target~foo(args).
Function and Mutator Definitions:

Both fn and mt are considered pure in the sense that they cannot access or modify state outside their defined target.
Mutable references cannot be passed as arguments to functions, emphasizing Krama's controlled approach to state mutation.

Type Definitions and Data Structures:

Krama allows for the definition of custom data structures (referred to as shape) and presumably supports generic programming, as indicated by the use of Option<Token> and List<T> in examples.
Lifecycle and Scope Management:

The language provides mechanisms (own, <- operator) for detailed management of variable lifecycles, allowing developers to control exactly when and where variables are allocated and freed.

## EBNF

```ebnf
(* Extended EBNF for Krama with 'target' and 'shape' constructs *)

program = { statement };

statement = variable_declaration
           | function_definition
           | mutator_definition
           | shape_definition
           | target_definition
           | expression
           | assignment ;

variable_declaration = ("let" | "mut"), identifier, ["=" expression] ;

function_definition = "fn", identifier, "(", [parameters], ")", ["->", type], block ;

mutator_definition = "mt", identifier, "(", [parameters], ")", block ;

shape_definition = "shape", identifier, "(", [field_definitions], ")" ;

field_definitions = field_definition { ",", field_definition } ;
field_definition = ("let" | "mut" | "art"), identifier, ":", type, ["->", expression] ;

target_definition = "target", "|", identifier, ":", type, "|", block ;

parameters = parameter { ",", parameter } ;
parameter = ["own"], identifier, ":", type ;

type = primitive_type | custom_type ;
primitive_type = "int" | "char" | "string" | "u64" | "bool" ;
custom_type = identifier ;

block = "{", { statement }, "}" ;

expression = identifier
            | literal
            | function_call
            | mutator_call
            | "(" expression ")" ;

function_call = identifier, ":", identifier, "(", [arguments], ")" ;
mutator_call = identifier, "~", identifier, "(", [arguments], ")" ;

arguments = expression { ",", expression } ;

assignment = (identifier, "<-") | "=", expression ;

literal = numeric_literal | string_literal | char_literal | boolean_literal ;
numeric_literal = digit, { digit } ;
string_literal = '"', { any_character }, '"' ;
char_literal = "'", character, "'" ;
boolean_literal = "true" | "false" ;

identifier = letter, { letter | digit | "_" } ;

(* Lexical elements *)
digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
letter = "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z"
       | "a" | "b" | "c" | "d" | "e" | "f" | "g"
       | "h" | "i" | "j" | "k" | "l" | "m" | "n"
       | "o" | "p" | "q" | "r" | "s" | "t" | "u"
       | "v" | "w" | "x" | "y" | "z" ;
character = <Any Unicode character> ;
any_character = character - ("\"" | "\'") ;
```
