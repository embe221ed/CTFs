#!/bin/bash

gunzip initramfs.cpio.gz
mkdir unpacked
cd unpacked
cpio -idv < ../initramfs.cpio 
