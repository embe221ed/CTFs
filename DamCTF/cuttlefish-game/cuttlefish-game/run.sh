#!/bin/bash

sudo rm initramfs.cpio.gz
set -e  # Crash on failures

sudo gcc -Wall -Wpedantic -Wextra -static -Os exp.c -o exp
sudo cp exp ./unpacked/exp

sudo ./pack.sh

terminator --new-tab -e 'qemu-system-x86_64 \
    -kernel "bzImage" \
    -m 128 \
    -initrd "initramfs.cpio.gz" \
    -nographic \
    -monitor none \
    -net none \
    -no-reboot \
    -append "console=ttyS0" \
    -cpu kvm64,+smep,+smap \
    -s'
