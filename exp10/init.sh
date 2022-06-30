#!/bin/bash
rmmod filter.ko # 首次加载内核，直接make，无需执行本行
make clean
make
insmod filter.ko
g++ test.c
./a.out

