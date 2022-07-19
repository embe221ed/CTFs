#!/usr/bin/env bash

for LIB in {libc.so.6,libstdc++.so.6};
do
    patchelf --replace-needed $LIB $(pwd)/$LIB madcore;
done

patchelf --set-interpreter $(pwd)/ld-linux-x86-64.so.2 madcore
