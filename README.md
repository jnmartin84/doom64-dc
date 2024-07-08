# Doom 64 for Dreamcast 

# playing guide
A is attack, b is use, x is weapon backward, y is weapon forward, start is start (bring up menu), d-pad/analog stick move, L trig strafe left (analog sensitive), R trig strafe right (analog sensitive), L+R trig together for automap, again for line map, third time back to game

# build guide

**Pre-reqs:**

Windows + WSL or Cygwin or Mingw and host `GCC`... or ... Ubuntu 22.04 host or VM install with `build-essential`/`GCC` installed

A host GCC install and a full working Dreamcast/KallistiOS toolchain (https://dreamcast.wiki/Getting_Started_with_Dreamcast_development).

**Repo contents**

Whatever the directory you cloned this github repo to is named and wherever it is located, it will be referred to in this document as

`doom64-dc`

This guide will assume that you cloned it into your home directory. 

If you need to get to the top level of the repo, it will say

    cd ~/doom64-dc

Under doom64-dc, you will find the following files and directories

    doom64-dc/
    -- README.md (you're reading it right now)
    -- wadtool/ (the tool that builds texture and WAD files from Doom 64 ROM)
    -- selfboot/
    ---- ogg/
    ------ *.ogg (all of the music tracks as 44khz mono OGG)
    ---- sfx/
    ------ *.wav (all of the game sfx as 22khz ADPCM WAV)
    ---- vq/ (where the sprite sheet for non enemy sprites ends up)
    ------ a (placeholder file for non-empty directory)


How to generate Dreamcast files:

Check that your doom64 ROM is the correct one

The below is the expected md5sum output

    md5sum doom64.z64
    b67748b64a2cc7efd2f3ad4504561e0e doom64.z64

Now copy your `doom64.z64` file to the `wadtool` directory.

Go to the `wadtool` directory in a terminal. Build the `wadtool`. Run the `wadtool`.

    cd ~/doom64-dc/wadtool
    ./build.sh
    ./wadtool

This should take a minute or less to run depending on your processor and disk speed.

When it is complete, you will now have the following new files in the `wadtool` directory:

    non_enemy.tex
    pow2.wad
    alt.wad

Copy these files to the selfboot directory one level up:

    cp non_enemy.tex ../selfboot/vq
    cp pow2.wad ../selfboot
    cp alt.wad ../selfboot

You now have all of the updated files required to run Doom 64 for Dreamcast in the places they need to be.

Go to the repo directory and compile it like any other KallistiOS project. Make sure you source your KOS environment first.
Build targets exist to create disc images for CD and Dreamshell SD ISO Loader.

    cd ~/doom64-dc
    make clean
    make

If you have `mkdcdisc` installed, you can use the `cdi` build target to create a self-booting CDI.

    cd ~/doom64-dc
    make cdi

If you have both `mksdiso` and `mkdcdisc` installed, you can use the `sdiso` build target to create an ISO suitable for loading from SD card with Dreamshell ISO loader.

    cd ~/doom64-dc
    make sdiso

Good luck.
