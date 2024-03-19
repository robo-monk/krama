import subprocess as sp
from uuid import uuid4
import os
import re

def make_tmp_file(content, id, ext):
    filename = f"tmp/{id}{ext}"
    with open(filename, 'w') as f:
        f.write(content)
    return filename

def del_tmp_file(filename):
    os.remove(filename)

def run_compiler_command(infile, outfile):
    # Assuming '../bin/krama' is the executable and 'compile' is an argument
    result = sp.run(['../bin/krama', 'compile', infile, outfile], capture_output=True, text=True)
    return result

def expect_compiler_error(content, expected_error):
    # Error looking like [compiler] Error! ...
    result = run_compiler(content)
    assert expected_error in result.stderr
        # or expected_error in result.stdout
    # assert re.search(r'\[compiler\] Error!.*', result.stderr) is not None
    # assert re.search(expected_error, result.stderr) is not None


def run_compiler(content):
    id = str(uuid4())
    krama_file = make_tmp_file(content, id, '.kr')
    try:
        print("\n---\n Running compiler with file: " + krama_file + "\n---")
        result = run_compiler_command(krama_file, "./tmp/out_" + id + '.c')
        print(result.stdout)
        print("Error: " + result.stderr)
        return result
    finally:
        try:
            print("Deleting file: " + krama_file)
            print("Deleting file: " + "tmp/out_" + id + '.c')
            del_tmp_file(krama_file)
            del_tmp_file("tmp/out_" + id + '.c')
        except:
            pass



def __array_test_compiler_errors(expected_compiler_error):
    for content, expected_error in expected_compiler_error:
        expect_compiler_error(content, expected_error)

def test_undeclared_variable_write():
    expected_compiler_error = [
    (
    """
    a = 5
    """, "tried to write an undeclared variable 'a'"),

    ("""
    let a = 5
    a + b
    """, "tried to read from undeclared variable 'b'"),


    ]
    __array_test_compiler_errors(expected_compiler_error)

def test_undeclared_variable_read():
    expected_compiler_error = [
    ("""
    let a = 5
    a + b
    """, "tried to read from undeclared variable 'b'"),
    ]
    __array_test_compiler_errors(expected_compiler_error)

def test_redeclared_variable():
    expected_compiler_error = [
    (
    """
    let hello = 5
    let hello = 6
    """, "redecleration of variable 'hello'"),
    ]
    __array_test_compiler_errors(expected_compiler_error)


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
    """, "ambigious return types"),
    ]
    __array_test_compiler_errors(expected_compiler_error)

def test_ambigious_bin():
    expected_compiler_error = [
    (
    """
    def hello(a: i32, b: f64) {
        a + b
    }
    """, "ambigious binary operation"),
    ]
    __array_test_compiler_errors(expected_compiler_error)



# def test_inf_recursion_bin():
#     expected_compiler_error = [
#     (
#     """
#     def hello() {
#     }
#     """, "ambigious binary operation"),
#     ]
#     __array_test_compiler_errors(expected_compiler_error)
