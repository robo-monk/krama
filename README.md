## TODOs

- [ ] rewrite hashmap myself

Syntax Ideas

```
let hello = 5

mut iterator = 0

let function_test = {
  scope {
    mut iterator = 0
  }

  post {
    if iterator == 0 {
      return;
    }

    print.post(str = iterator)

    function_test.post({
      iterator = iterator - 1
    })
  }
}

function_test.post({
  iterator = 5
});


let put = {
  ::string  as  value -> c::printf("%s", value);
  ::int     as  value -> c::printf("%d", value);
}

let echo = {
  ::() as value -> put::int(value)
}

echo::({5 +  4}) // 9

function_test
  .scope({
    mut hello = "im also there?"
  })
  .post({
    put::string("hello")
  })



proc put {
  ::string  => c::printf("%s", value);
  ::int     =>
}


// everything is a statement
let a = 5; // evaluates to 5
let b = 3;

pragma while {
  ::(@condition<bool>, { @code }) {
    if @condition {
      @code
    }
    while(@condition, @code)
  }

  do::(@condition<bool>, { @code }) {
    @code
    while(@condition, @code)
  }
}

pragma pow {
  ::(@a<u32>, @b<u32>) {
    mut i = 0
    while (i < @b, {
      @a *= @a
      i++
    })
    @a
  }
}


pragma u32 {
  @var: :: {
    @var.type = 'u32'
  }

  ::add(@a: u32, @b: u32) {

  }
}

pragma + {
  @a:u32 :: @b:u32 {

    u32::add(@a, @b)

  }
}

pragma complex {
  ::real
}

int::add(a, b)
5 + 5



pragma mut {
  ::f64 {

  }
}


let.f64 :pi   = 3.1415;
mut.f64 :test = 4.3;


pragma complex {
  ::@var {}

  mut f64 :re   = 0;
  mut f64 :im   = 0;
}

complex :a = {
}


```
