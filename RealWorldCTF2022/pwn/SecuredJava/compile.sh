#!/bin/zsh
cd ctf
javac -cp . -proc:none com/h4ck1t/ctf/annotationprocessor/log/*.java
jar cfv ../dep.jar ./**/*
cd ..
