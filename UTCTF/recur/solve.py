flag = bytearray.fromhex("b5d822e2a9c57176")[::-1]
flag += bytearray.fromhex("5a90bfb22892f173")[::-1]
flag += bytearray.fromhex("da9021b3a6fc7776")[::-1]
flag += bytearray.fromhex("38cfb56f")[::-1]
print(flag)
results = {}


def recurrence(arg1):
    if results.get(arg1):
        return results[arg1]

    if (arg1 == 0):
        iVar1 = 3
    else:
        if (arg1 == 1):
            iVar1 = 5
        else:
            iVar2 = recurrence(arg1 - 2)
            iVar1 = recurrence(arg1 - 1)
            iVar1 = iVar2 * 3 + iVar1 * 2

    results[arg1] = iVar1
    return iVar1 & 0xff

for i in range(len(flag)):
    var = recurrence(i*i)
    print(chr(flag[i]^var), end='')
