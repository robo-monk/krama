import testing_utils as tu

def test_undeclared_variable_write():
    expected_compiler_error = [
    (
    """
    a = 5
    """, "tried to write an undeclared variable 'a'"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)

def test_undeclared_variable_read():
    expected_compiler_error = [
    ("""
    let a: i32 = 5
    a + b
    """, "tried to read from undeclared variable 'b'"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)

def test_redeclared_variable():
    expected_compiler_error = [
    (
    """
    let hello: i32 = 5
    let hello: f64 = 6
    """, "redecleration of variable 'hello'"),
    ]
    tu.array_test_compiler_errors(expected_compiler_error)
