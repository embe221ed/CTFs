#!/bin/bash

gunzip initramfs.cpio.gz
cd unpacked
cpio -idv < ../initramfs.cpio 
