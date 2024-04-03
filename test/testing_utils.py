import subprocess as sp
from uuid import uuid4
import os
import re

def make_tmp_file(content, id, ext):
    filename = f"./tmp/{id}{ext}"
    with open(filename, 'w', encoding="utf-8") as f:
        f.write(content)
    return filename

def del_tmp_file(filename):
    os.remove(filename)

def run_compiler_command(infile, outfile):
    # Assuming '../bin/krama' is the executable and 'compile' is an argument
    # result = sp.run(['../bin/krama', 'compile', infile, outfile, "--std-path", "../std.kr"], capture_output=True, text=False)
    result = sp.run(['../bin/krama', 'compile', infile, outfile, "--no-std"], capture_output=True, text=False)
    stdout = result.stdout.decode('utf-8', errors='replace')
    stderr = result.stderr.decode('utf-8', errors='replace')
    return (stdout, stderr, result)

def expect_compiler_error(content, expected_error):
    # Error looking like [compiler] Error! ...
    (stdout, stderr, result), file_content = run_compiler(content)
    assert expected_error in stderr

def expect_in_outc(content, expected):
    # Error looking like [compiler] Error! ...
    (stdout, stderr, result), file_content = run_compiler(content)
    assert expected in file_content


def run_compiler(content):
    id = str(uuid4())
    krama_file = make_tmp_file(content, id, '.kr')
    try:
        print("\n---\n Running compiler with file: " + krama_file + "\n---")
        (stdout, stderr, result) = run_compiler_command(krama_file, "./tmp/out_" + id + '.c')
        # print(stdout)
        # print("Error: " + stderr)
        outc_content = ""
        if result.returncode == 0:
            with open("./tmp/out_" + id + '.c', 'r', encoding="utf-8") as f:
                outc_content = f.read()
                print("out c will be ", outc_content)

        return (stdout, stderr, result), outc_content
    except:
        print("Error???/")
    finally:
        try:
            print("Deleting file: " + krama_file)
            print("Deleting file: " + "tmp/out_" + id + '.c')
            # del_tmp_file(krama_file)
            # del_tmp_file("./tmp/out_" + id + '.c')
        except:
            pass


def array_test_compiler_errors(expected_compiler_error):
    for content, expected_error in expected_compiler_error:
        expect_compiler_error(content, expected_error)

def array_test_compiler_inference(expected_inference_content):
    for content, expected in expected_inference_content:
        expect_in_outc(content, expected)
