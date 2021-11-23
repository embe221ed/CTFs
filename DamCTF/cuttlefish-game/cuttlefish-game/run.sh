#!/bin/bash

set -e  # Crash on failures

gcc -Wall -Wextra -Wpedantic -Werror -static -Os exp.c -o exp
cp exp ./unpacked/exp

./pack.sh

qemu-system-x86_64 \
    -kernel "bzImage" \
    -m 128 \
    -initrd "initramfs.cpio.gz" \
    -nographic \
    -monitor none \
    -net none \
    -no-reboot \
    -append "console=ttyS0" \
    -cpu kvm64,+smep,+smap
