#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
if ! [ -f $SCRIPT_DIR/wadtool ]; then
  echo "Compiling wadtool"
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/encode.c -o $SCRIPT_DIR/encode.o
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/imgproc.c -o $SCRIPT_DIR/imgproc.o
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/wadtool.c -o $SCRIPT_DIR/wadtool.o
  gcc $SCRIPT_DIR/encode.o $SCRIPT_DIR/imgproc.o $SCRIPT_DIR/wadtool.o -o $SCRIPT_DIR/wadtool
fi
echo "Running wadtool"
time $SCRIPT_DIR/wadtool $SCRIPT_DIR/doom64.z64 $SCRIPT_DIR/../selfboot
echo "Generated data files in specified selfboot directory."
echo "Done."
