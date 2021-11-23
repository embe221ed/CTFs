FROM ubuntu:20.10

RUN apt-get update && apt-get install -y vim tree gdb g++ make git socat

RUN git clone https://github.com/pwndbg/pwndbg.git && cd pwndbg && ./setup.sh

RUN pip install pwntools
