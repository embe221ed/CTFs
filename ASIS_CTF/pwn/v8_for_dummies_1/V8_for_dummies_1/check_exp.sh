#!/bin/sh
D8=~/v8/v8/out/x64.debug/d8
FLAGS="--allow-natives-syntax --trace-turbo --trace-deopt"
CMD="${D8} ${FLAGS}"
if [ "$#" -ne 1 ];
then
   ${CMD} exp.js
else
  if [ "$1" = "run" ];
  then
    ${CMD}
  else
    echo ${CMD}
  fi
fi
