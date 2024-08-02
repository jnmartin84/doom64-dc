# Doom 64 for Dreamcast 


You have to do a tiny bit of actual work to get this going. If you don't have 30 minutes to spare, just go play on Steam and call it a day.


# playing guide

    A is attack
    B is use
    X is weapon backward
    Y is weapon forward
    combined press of X and Y brings up automap and cycles through (second press goes to line map, third press back to game)
    D-PAD/Analog Stick is move
    L trigger is strafe left (analog sensitive)
    R trigger strafe right (analog sensitive)
    START is start (bring up menu)

# build guide

**Pre-requisites**

The build is known to work on the following platforms as of the current commit:
    Debian (version?)
    Ubuntu 22.04
    Windows 11 - Cygwin 64-bit
    Windows (version? - DreamSDK

It should work on most other Linux environments.
    
You will need a host/native GCC install and a full working Dreamcast/KallistiOS toolchain install (https://dreamcast.wiki/Getting_Started_with_Dreamcast_development).



**Repo contents**

Whatever the directory you cloned this github repo to is named and wherever it is located, it will be referred to in this document as

`doom64-dc`

This guide will assume that you cloned it into your home directory. 

If you need to get to the top level of the repo, it will say

    cd ~/doom64-dc

Under doom64-dc, you will find

    doom64-dc/
    -- README.md (you're reading it right now)
    -- Makefile, Makefile.kos (how it gets built)
    -- wadtool/ (the tool that builds texture and WAD files from Doom 64 ROM)
    -- selfboot/ (all files needed to make a bootable CD image)
    ---- ogg/ (all of the music tracks as 44khz mono OGG)
    ---- sfx/ (all of the game sfx as 22khz ADPCM WAV)
    ---- vq/ (where the sprite sheet for non enemy sprites ends up)



**How to generate Doom 64 disc image**

Somehow acquire a Doom 64 ROM in Z64 format.

Check that your Doom 64 ROM is the correct one.

The below is the expected md5sum output

    md5sum doom64.z64
    b67748b64a2cc7efd2f3ad4504561e0e doom64.z64

Now place a copy of `doom64.z64` in the `wadtool` directory.

Go to the repo directory and compile it like any other KallistiOS project. Make sure you source your KOS environment first. It is starting to look like there is an issue when it is built at -O2 without lto. Modify `environ.sh` so KOS_CFLAGS has `-O3 -flto=auto` instead of `-O2` before you build.

To build the source into an ELF file, run `make`.

    source /opt/toolchains/dc/kos/environ.sh
    cd ~/doom64-dc
    make clean
    make

As part of the build, `Make` will automatically build and run `wadtool`.

This should take a minute or less to run depending on your processor and disk speed.

The first terminal output you see should match the following except for the time values (the first time you run `make`):

    Compiling wadtool
    Running wadtool
    
    real   0m20.368s
    user   0m19.836s
    sys    0m0.233s
    Generated data files in specified selfboot directory.
    Done.

Subsequent runs will not rebuild `wadtool` but start at `Running wadtool` or skip that too if the generated files already exist in `selfboot`.

When it is complete, you will now have the following new files in the `~/doom64-dc/selfboot` directory:

    alt.wad
    pow2.wad
    vq/non_enemy.tex

You now have all of the updated files required to run Doom 64 for Dreamcast in the places they need to be.

If you have `mkdcdisc` installed, you can use the `cdi` build target to create a self-booting CDI.

    cd ~/doom64-dc
    make cdi

If you have both `mksdiso` and `mkdcdisc` installed, you can use the `sdiso` build target to create an ISO suitable for loading from SD card with Dreamshell ISO loader.

    cd ~/doom64-dc
    make sdiso

If you are trying to use any other tool, you are on your own.

Good luck. :-)
