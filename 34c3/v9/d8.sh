#!/bin/bash

V8_DIR=/opt/v8/v8/out.gn
FLAGS="\
    --allow-natives-syntax \
    --trace-turbo \
"

if [[ $1 != turbolizer ]]
then
    rm turbo* 2>/dev/null
fi

if [[ $# -gt 1 ]]
then
    for ARG in ${@:2}; do
        FLAGS+=" ${ARG}"
    done
fi

if [[ $1 == "debug" ]]
then
    $V8_DIR/x64.debug/d8 $FLAGS
elif [[ $1 == "release" ]]
then
    $V8_DIR/x64.release/d8 $FLAGS
elif [[ $1 == "gdbr" ]]
then
    gdb -ex=r -x script.gdb --args $V8_DIR/x64.release/d8 $FLAGS --
elif [[ $1 == "gdbd" ]]
then
    gdb -ex=r -x script.gdb --args $V8_DIR/x64.debug/d8 $FLAGS --
elif [[ $1 == "turbolizer" ]]
then
    terminator --new-tab -x 'cd /opt/v8/v8/tools/turbolizer && python3 -m http.server 8080'
else
    FLAGS+=" $1"
    $V8_DIR/x64.release/d8 $FLAGS
fi
