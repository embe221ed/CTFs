#!/usr/bin/env python3
canary = 0x90f0e3f69854adb8

state = canary

def nextbit():
    global state
    bit = 1 & (state >> 0x3f ^ state >> 0x3d ^ state >> 0x3c ^ state >> 0x3a ^ 1)
    state = bit | state << 1 & 0xffff_ffff_ffff_ffff
    return bit

def rewind(got):
    global state
    state >>= 1
    bit = got ^ state >> 0x3d ^ state >> 0x3c ^ state >> 0x3a ^ 1
    state = (state | bit << 0x3f) & 0xffff_ffff_ffff_ffff

def rand(n):
    ret = 0
    for _ in range(n):
        ret <<= 1
        ret |= nextbit()
    return ret

print(hex(canary))

main = rand(12)
syscalls = rand(12)
guard = rand(12)
basic = rand(12)
game = rand(12)
res = rand(12)
debug = rand(12)

print("main:", hex(main))
print("syscalls:", hex(syscalls))
print("guard:", hex(guard))
print("basic:", hex(basic))
print("game:", hex(game))
print("res", hex(res))
print("debug", hex(debug))

# recover
state = (res | game << 12 | basic << 24 | guard << 36 | syscalls << 48 | main << 60) & 0xffff_ffff_ffff_ffff

for _ in range(6 * 12):
    rewind(state & 1)

print(hex(state))
