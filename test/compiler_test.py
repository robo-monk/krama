import subprocess as sp
from uuid import uuid4
import os
import re

def make_tmp_file(content, id, ext):
    filename = f"./tmp/{id}{ext}"
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
    result, file_content = run_compiler(content)
    assert expected_error in result.stderr
        # or expected_error in result.stdout
    # assert re.search(r'\[compiler\] Error!.*', result.stderr) is not None
    # assert re.search(expected_error, result.stderr) is not None

def expect_in_outc(content, expected):
    # Error looking like [compiler] Error! ...
    result, file_content = run_compiler(content)
    assert expected in file_content
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
        outc_content = ""
        if result.returncode == 0:
            with open("./tmp/out_" + id + '.c', 'r') as f:
                outc_content = f.read()

        return result, outc_content
    finally:
        try:
            print("Deleting file: " + krama_file)
            print("Deleting file: " + "tmp/out_" + id + '.c')
            del_tmp_file(krama_file)
            del_tmp_file("./tmp/out_" + id + '.c')
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
    let a: i32 = 5
    a + b
    """, "tried to read from undeclared variable 'b'"),


    ]
    __array_test_compiler_errors(expected_compiler_error)

def test_undeclared_variable_read():
    expected_compiler_error = [
    ("""
    let a: i32 = 5
    a + b
    """, "tried to read from undeclared variable 'b'"),
    ]
    __array_test_compiler_errors(expected_compiler_error)

def test_redeclared_variable():
    expected_compiler_error = [
    (
    """
    let hello: i32 = 5
    let hello: f64 = 6
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
    """, "ambiguous"),
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

def test_wrong_ass():
    expected_compiler_error = [
    (
    """
    def hello(a: i32) {
        let b: f64 = a
    }
    """, "mismatched types"),
    ]
    __array_test_compiler_errors(expected_compiler_error)


def __array_test_compiler_inference(expected_inference_content):
    for content, expected in expected_inference_content:
        expect_in_outc(content, expected)


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
    __array_test_compiler_inference(e)



def test_binop_inference():
    e = [
    (
    """
    def add(a: i32, b: i32) {
        a+b
    }
    """, "i32 add("),
    ]
    __array_test_compiler_inference(e)


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
    __array_test_compiler_inference(e)



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
    __array_test_compiler_inference(e)

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
    __array_test_compiler_inference(e)
