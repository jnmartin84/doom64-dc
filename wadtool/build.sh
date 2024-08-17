#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
if ! [ -f $SCRIPT_DIR/wadtool ]; then
  echo "Compiling wadtool"
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/decodes.c -o $SCRIPT_DIR/decodes.o
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/encode.c -o $SCRIPT_DIR/encode.o
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/imgproc.c -o $SCRIPT_DIR/imgproc.o
  gcc -Wno-unused-result -O3 -c $SCRIPT_DIR/wadtool.c -o $SCRIPT_DIR/wadtool.o
  gcc $SCRIPT_DIR/decodes.o $SCRIPT_DIR/encode.o $SCRIPT_DIR/imgproc.o $SCRIPT_DIR/wadtool.o -o $SCRIPT_DIR/wadtool
fi

if [ -f $SCRIPT_DIR/../selfboot/alt.wad ]; then
  if [ -f $SCRIPT_DIR/../selfboot/pow2.wad ]; then
    if [ -f $SCRIPT_DIR/../selfboot/vq/non_enemy.tex ]; then
      echo "Game data files have already been generated; exiting."
      exit 0
    fi
  fi
fi

echo "Running wadtool"
time $SCRIPT_DIR/wadtool $SCRIPT_DIR/doom64.z64 $SCRIPT_DIR/../selfboot
echo "Generated data files in specified selfboot directory."
echo "Done."
