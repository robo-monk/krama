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

def run_compiler_command_no_std(infile, outfile):
    # Assuming '../bin/krama' is the executable and 'compile' is an argument
    # result = sp.run(['../bin/krama', 'compile', infile, outfile, "--std-path", "../std.kr"], capture_output=True, text=False)
    result = sp.run(['../bin/krama', 'compile', infile, outfile, "--no-std"], capture_output=True, text=False)
    stdout = result.stdout.decode('utf-8', errors='replace')
    stderr = result.stderr.decode('utf-8', errors='replace')
    return (stdout, stderr, result)

def run_compiler_command_w_std(infile, outfile):
    # Assuming '../bin/krama' is the executable and 'compile' is an argument
    # result = sp.run(['../bin/krama', 'compile', infile, outfile, "--std-path", "../std.kr"], capture_output=True, text=False)
    result = sp.run(['../bin/krama', 'compile', infile, outfile, "--std-path", "../std.kr"], capture_output=True, text=False)
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

def expect_in_out_executed(content, expected):
    (stdout, stderr, result) = compile_and_run(content)
    assert stdout is not None
    assert expected in stdout


def compile_and_run(content):
    id = str(uuid4())
    krama_file = make_tmp_file(content, id, '.kr')
    outc_file = "./tmp/out_" + id + '.c'
    outa_file = "./tmp/out_" + id + '.a'
    try:
        print("\n---\n Running compiler with file: " + krama_file + "\n---")
        (stdout, stderr, result) = run_compiler_command_w_std(krama_file, outc_file)
        if stderr is not None:
            print(stderr)

        if result.returncode == 0:
            sp.run(["clang", outc_file, "-o", outa_file])
            result = sp.run([f"./{outa_file}"], capture_output=True, text=False)
            stdout = result.stdout.decode('utf-8', errors='replace')
            stderr = result.stderr.decode('utf-8', errors='replace')
            return (stdout, stderr, result)
    except:
        print(f"Error compiling and running {content}")

    return (None, None, None)


def run_compiler(content):
    id = str(uuid4())
    krama_file = make_tmp_file(content, id, '.kr')
    try:
        print("\n---\n Running compiler with file: " + krama_file + "\n---")
        (stdout, stderr, result) = run_compiler_command_no_std(krama_file, "./tmp/out_" + id + '.c')
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

    return (None, None, None), None


def array_test_compiler_errors(expected_compiler_error):
    for content, expected_error in expected_compiler_error:
        expect_compiler_error(content, expected_error)

def array_test_compiler_inference(expected_inference_content):
    for content, expected in expected_inference_content:
        expect_in_outc(content, expected)
