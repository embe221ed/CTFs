#/bin/bash
socat tcp-l:4000,reuseaddr,fork exec:./wrapper.sh
