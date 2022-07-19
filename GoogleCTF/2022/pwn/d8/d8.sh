#!/bin/bash

V8_DIR=/opt/v8/v8/out
FLAGS="--allow-natives-syntax"

if [[ $# -gt 1 ]]
then
    for ARG in ${@:2}; do
        FLAGS+=" ${ARG}"
    done
fi

if [[ $1 == "help" ]]
then
    echo "Usage: ./d8.sh FLAGS"
    echo -e "\t[d|r]build - [re]generate blob.bin with (exp.js FLAGS) and run challenge with (blob.bin)"
    echo -e "\t[debug|release] - run challenge with (blob.bin)"
    echo -e "\td8[d|r] - run d8 with (FLAGS)"
    echo -e "\tgdb[d|r] - run challenge under gdb with (blob.bin)"
elif [[ $1 == "dbuild" ]]
then
    $V8_DIR/debug/g3n exp.js $FLAGS
    $V8_DIR/debug/challenge blob.bin
elif [[ $1 == "rbuild" ]]
then
    $V8_DIR/release/g3n exp.js $FLAGS
    $V8_DIR/release/challenge blob.bin
elif [[ $1 == "debug" ]]
then
    $V8_DIR/debug/challenge blob.bin
elif [[ $1 == "release" ]]
then
    $V8_DIR/release/challenge blob.bin
elif [[ $1 == "d8d" ]]
then
    # $V8_DIR/debug/d8 $FLAGS
    gdb -ex=r -x script.gdb --args $V8_DIR/debug/d8 $FLAGS --
elif [[ $1 == "d8r" ]]
then
    # $V8_DIR/release/d8 $FLAGS
    gdb -ex=r -x script.gdb --args $V8_DIR/debug/d8 $FLAGS --
elif [[ $1 == "gdbr" ]]
then
    gdb -ex=starti -x script.gdb --args $V8_DIR/release/challenge blob.bin --
elif [[ $1 == "gdbd" ]]
then
    gdb -ex=starti -x script.gdb --args $V8_DIR/debug/challenge blob.bin --
else
    FLAGS+=" $1"
    $V8_DIR/release/challenge blob.bin
fi
