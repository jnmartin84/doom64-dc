#!/bin/sh
gcc -O3 -c encode.c -o encode.o
gcc -O3 -c imgproc.c -o imgproc.o
gcc -O3 -c wadtool.c -o wadtool.o
gcc encode.o imgproc.o wadtool.o -o wadtool.exe
