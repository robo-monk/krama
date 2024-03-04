# Machine for Evaluting Textual Heaps (METH)

```
$a = 44 + 3 + 333 + (390 * 3)
```

# Reactive

```
define mut string cout
define fn printer = {
    printf($cout)
}

define fn await_user_input = {
    define mut string user_input
    define mut bool await_user_input = { $user_input_char != KEY_RETURN }

    if $await_user_input:
        user_input << $user_input_char

    user_input
}

define fn parse_user_input = {
    define const user_input = await_user_input()
    define mut int index = 0

    if user_input.length == 0:
        cout += "no input"
        exit()

}
```
