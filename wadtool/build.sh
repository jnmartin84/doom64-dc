#!/bin/bash
echo "Compiling wadtool"
gcc -O3 -c ./encode.c -o ./encode.o
gcc -O3 -c ./imgproc.c -o ./imgproc.o
gcc -O3 -c ./wadtool.c -o ./wadtool.o
gcc ./encode.o ./imgproc.o ./wadtool.o -o ./wadtool
echo "Running wadtool"
time ./wadtool
echo "Copying generated data files to selfboot directory"
cp alt.wad ../selfboot
cp pow2.wad ../selfboot
cp non_enemy.tex ../selfboot/vq
echo "Done."
