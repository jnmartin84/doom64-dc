# Doom 64 for Dreamcast 

# playing guide

    A is attack
    B is use
    X is weapon backward
    Y is weapon forward
    START is start (bring up menu)
    D-PAD/Analog Stick is move
    L trigger is strafe left (analog sensitive)
    R trigger strafe right (analog sensitive)
    combined press of L trigger and R trigger brings up automap and cycles through (second press goes to line map, third press back to game)

# build guide

**Pre-requisites**

The build has been done on the following platforms successfully:

    Windows 10/11 with Cygwin
    Ubuntu 22.04
    Debian (unknown version)

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
    -- wadtool/ (the tool that builds texture and WAD files from Doom 64 ROM)
    -- selfboot/ (all files needed to make a bootable CD image)
    ---- ogg/ (all of the music tracks as 44khz mono OGG)
    ---- sfx/ (all of the game sfx as 22khz ADPCM WAV)
    ---- vq/ (where the sprite sheet for non enemy sprites ends up)


**How to generate Dreamcast files**

Somehow acquire a Doom 64 ROM in Z64 format.

Check that your Doom 64 ROM is the correct one.

The below is the expected md5sum output

    md5sum doom64.z64
    b67748b64a2cc7efd2f3ad4504561e0e doom64.z64

Now place a copy of `doom64.z64` in the `wadtool` directory.

Go to the `wadtool` directory in a terminal. Run the `build.sh` script. This builds the tool, runs it and copies the outputs to the correct location.

    cd ~/doom64-dc/wadtool
    ./build.sh

This should take a minute or less to run depending on your processor and disk speed.

You should see terminal output that matches the following except for the time values:

    Compiling wadtool
    Running wadtool
    
    real   0m20.368s
    user   0m19.836s
    sys    0m0.233s
    Copying generated data files to selfboot directory
    Done.

When it is complete, you will now have the following new files in the `~/doom64-dc/selfboot` directory:

    alt.wad
    pow2.wad
    vq/non_enemy.tex

You now have all of the updated files required to run Doom 64 for Dreamcast in the places they need to be.

**Building Doom 64 disc image**

Go to the repo directory and compile it like any other KallistiOS project. Make sure you source your KOS environment first.

   source /opt/toolchains/dc/kos/environ.sh

To build the source into an ELF file, run `make`.

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
