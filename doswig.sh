#!/bin/sh

echo "--Swigging..."
swig -v -python linac.i
echo "--Compiling..."
gcc -fPIC -c \
    linac_param.c \
    doublecompress.c \
    filter.c \
    step_llrf.c \
    state_space_top.c \
    beam_based_feedback.c \
    dynamic_noise.c \
    linac_wrap.c \
    --std=gnu99 -I/usr/include/python2.7 -I/usr/include/numpy
echo "--Linking..."
ld -shared \
    linac_param.o \
    doublecompress.o \
    filter.o \
    step_llrf.o \
    state_space_top.o \
    beam_based_feedback.o \
    dynamic_noise.o \
    linac_wrap.o \
    -o _linac.so
echo "--Done, unless there were errors..."