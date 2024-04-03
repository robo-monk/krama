import testing_utils as tu

# inference test
def test_ambigious_if():
    expected_compiler_error = [
    (
    """
    def hello(a: i32, b: f64) {
        if (1) {
            a
        } else {
            b
        }
    }
    """, "ambiguous"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)

def test_ambigious_bin():
    expected_compiler_error = [
    (
    """
    def hello(a: i32, b: f64) {
        a + b
    }
    """, "ambigious binary operation"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)

def test_wrong_ass():
    expected_compiler_error = [
    (
    """
    def hello(a: i32) {
        let b: f64 = a
    }
    """, "mismatched types"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)



def test_basic_inference():
    e = [
    (
    """
    def hello(x: i32) {
       x
    }
    """, "i32 hello(i32"),

    (
    """
    def hello(x: f64) {
       x
    }
    """, "f64 hello("),
    ]
    tu.array_test_compiler_inference(e)



def test_binop_inference():
    e = [
    (
    """
    def add(a: i32, b: i32) {
        a+b
    }
    """, "i32 add("),
    ]
    tu.array_test_compiler_inference(e)


def test_recursive_inference():
    e = [
    (
    """
    def as_f64(a: f64) {
        a
    }

    def fac(a: f64) {
        if (a < 2) {
            as_f64(1)
        } else {
            a * fac(a - 1)
        }
    }
    """, "f64 fac("),
    # (
    # """
    #     def tricky(b: i32) {
    #         let c: f64 = 0
    #         c
    #     }
    # """, "f64 tricky(")
    ]
    tu.array_test_compiler_inference(e)



def test_special_inference():
    e = [
    (
    """
    def nothing() {
        #c printf
    }
    """, "void nothing("),
    (
    """
    def aloc(size: i32) {
        let adr: ptr = 0
        #c adr = malloc(adr)
        adr
    }
    """, "ptr aloc(i32 size"
    )
    ]
    tu.array_test_compiler_inference(e)

def test_namespace_inference():
    e = [
    (
    """
    def |x: i32| add(a: i32) {
        x + a
    }

    let one: i32 = 1
    one:add(5)
    """, "i32 add("),
    ]
    tu.array_test_compiler_inference(e)
