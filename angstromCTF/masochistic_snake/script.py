#!/usr/bin/env python3

import gdb
import string


def is_hex(pattern):
    """Return whether provided string is a hexadecimal value."""
    if not pattern.startswith("0x") and not pattern.startswith("0X"):
        return False
    return len(pattern)%2==0 and all(c in string.hexdigits for c in pattern[2:])

def to_unsigned_long(v):
    mask = (1 << 64) - 1
    return int(v.cast(gdb.Value(mask).type)) & mask

def parse_address(address):
    if is_hex(address):
        return int(address, 16)
    return to_unsigned_long(gdb.parse_and_eval(address))

def read_memory(addr, length):
    return gdb.selected_inferior().read_memory(addr, length).tobytes()

def dumps_board(addr):
    b = read_memory(addr, 0x100)
    assert all(x in range(4) for x in b)
    s = ''.join(map(str, b))
    lines = (s[idx:idx+16] for idx in range(0, 256, 16))
    return f'{addr:#x}:\n' + '\n'.join(lines) + '\n'

class DumpBoardCommand(gdb.Command):
    @staticmethod
    def invoke(line, from_tty):
        argv = gdb.string_to_argv(line)
        assert len(argv) == 1
        addr = argv[0]
        addr = parse_address(addr)
        gdb.write(dumps_board(addr))
DumpBoardCommand('board', gdb.COMMAND_NONE, gdb.COMPLETE_LOCATION)