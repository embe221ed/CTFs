1. I can allocate up to 8 chunks
2. I probably cannot allocate any freed chunk


pwndbg> p/x 0x7f1f6f63b000 - 0x55884e4ec260
$2 = 0x29972114eda0


pwndbg> p/x 0x7f1b2fef2000 - 0x55d16fabe260
$1 = 0x2949c0433da0
