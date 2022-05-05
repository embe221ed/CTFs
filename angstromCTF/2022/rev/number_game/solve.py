#!/usr/bin/env python3
from pwn import *

exe = ELF('./number-game')

HOST, PORT = "challs.actf.co 31334".split()

def start():
    if args.REMOTE:
        return remote(HOST, PORT)
    if args.GDB:
        return gdb.debug([exe.path], gdbscript=gdbscript)
    else:
        return process([exe.path])

gdbscript = '''
tbreak main
continue
'''

io = start()

solution = 0x12b9b0a1
io.sendlineafter(b"number: ", str(solution).encode())

solution = 0x1e996cc9 - solution
io.sendlineafter(b"up? ", str(solution).encode())

solution = "the airspeed velocity of an unladen swallow"
io.sendlineafter(b"alphabet?\n", str(solution).encode())

io.interactive()

# actf{it_turns_out_you_dont_need_source_huh}
