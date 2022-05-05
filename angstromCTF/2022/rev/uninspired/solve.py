#!/usr/bin/env python3
import sys
from pwn import *

exe = ELF('./uninspired')

def start():
    if args.GDB:
        return gdb.debug([exe.path], gdbscript=gdbscript)
    else:
        return process([exe.path])

gdbscript = '''
tbreak main
continue
'''

def calculate(solution: list) -> bool:
    temp_list = [0] * 10
    for i in range(len(solution)):
        elem = solution[i] - 0x30
        temp_list[elem] = temp_list[elem] + 1

    for i in range(len(solution)):
        elem = solution[i] - 0x30
        if elem != temp_list[i]:
            return False
    else:
        return True


solution = [
    0x36, # 0
    0x32, # 1
    0x31, # 2
    0x30, # 3
    0x30, # 4
    0x30, # 5
    0x31, # 6
    0x30, # 7
    0x30, # 8
    0x30  # 9
]

io = start()

solution = b"".join(p8(_) for _ in solution)
io.recvline()
io.sendline(solution)

io.interactive()

# actf{ten_digit_numbers_are_very_inspiring}
