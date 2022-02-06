#!/bin/bash

if [[ $1 == "h" ]]
then
    echo -e "run script"
    echo -e "To:"
    echo -e "\t- build docker image run:\t\t./run.sh build"
    echo -e "\t- run shell inside container run:\t./run.sh sh"
    echo -e "\t- compile the binary run:\t\t./run.sh compile"
    echo -e "\t- start docker process run:\t\t./run.sh WHATEVA"
elif [[ $1 == "build" ]]
then
    cd docker
    docker build -t svme .
    cd ..
elif [[ $1 == "sh" ]]
then
    docker run -it --rm svme /bin/bash
elif [[ $1 == "compile" ]]
then
    cp docker/main.c simple-virtual-machine-C-master/src/vmtest.c &&\
    cd simple-virtual-machine-C-master &&\
    make clean &&\
    cmake . &&\
    make &&\
    cd ..
else
    docker run -it --rm -p 1337:1337 svme 
fi
