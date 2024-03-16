import subprocess as sp
from uuid import uuid4
import os
import re

def make_tmp_file(content, id, ext):
    filename = f"{id}{ext}"
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
    assert re.search(r'\[compiler\] Error!.*', result.stderr) is not None
    assert re.search(expected_error, result.stderr) is not None


def run_compiler(content):
    id = str(uuid4())
    krama_file = make_tmp_file(content, id, '.kr')
    try:
        result = run_compiler_command(krama_file, "out_" + id + '.c')
        return result
    finally:
        print("Deleting file: " + krama_file)
        # del_tmp_file(krama_file)
        # del_tmp_file("out_" + id + '.c')


expected_compiler_error = [
(
"""
a = 5
""", "tried to write an undeclared variable")
]

def test_undeclared_variable():
    for content, expected_error in expected_compiler_error:
        expect_compiler_error(content, expected_error)
