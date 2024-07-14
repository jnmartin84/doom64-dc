#!/bin/bash
echo "Compiling wadtool"
gcc -Wno-unused-result -O3 -c ./encode.c -o ./encode.o
gcc -Wno-unused-result -O3 -c ./imgproc.c -o ./imgproc.o
gcc -Wno-unused-result -O3 -c ./wadtool.c -o ./wadtool.o
gcc ./encode.o ./imgproc.o ./wadtool.o -o ./wadtool
echo "Running wadtool"
time ./wadtool ./doom64.z64 ../selfboot
echo "Generated data files in specified selfboot directory."
echo "Done."
