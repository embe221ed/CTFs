#!/bin/bash

SM_DIR=/opt/spidermonkey/spidermonkey/obj-debug-x86_64-pc-linux-gnu/dist/bin
FLAGS=""

if [[ $# -gt 1 ]]
then
    for ARG in ${@:2}; do
        FLAGS+=" ${ARG}"
    done
fi

if [[ $1 == "debug" ]]
then
    $SM_DIR/js $FLAGS
elif [[ $1 == "release" ]]
then
    $SM_DIR/js $FLAGS
elif [[ $1 == "gdbr" ]]
then
    gdb -ex=r -x script.gdb --args $SM_DIR/js $FLAGS --
elif [[ $1 == "gdbd" ]]
then
    gdb -ex=r -x script.gdb --args $SM_DIR/js $FLAGS --
else
    FLAGS+=" $1"
    $SM_DIR/js $FLAGS
fi
