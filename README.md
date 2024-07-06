# Doom 64 for Dreamcast 

# playing guide
A is attack, b is use, x is weapon backward, y is weapon forward, start is start (bring up menu), d-pad/analog stick move, l trig strafe left (analog sensitive), R trig strafe right (analog sensitive), l+R trig together for automap, again for line map, third time back to game

# build guide

**Pre-reqs:**

Ubuntu 22.04 host or VM install with `build-essential` package installed

( `sudo apt-get install build-essential` )

A host GCC install and a full working Dreamcast/KallistiOS toolchain (https://dreamcast.wiki/Getting_Started_with_Dreamcast_development).

Install `SLADE` (for Linux see https://flathub.org/apps/net.mancubus.SLADE)

Install `Doom64 EX` 

Install `rename` if you don't already have it
( `sudo apt-get install rename` )

Install `python3` and `python3-pip` if you don't already have them

( `sudo apt-get install python3 python3-pip ` )

Pip install Pillow
`pip3 install pillow`

Install `GIMP 2.x` 
( `sudo apt-get install gimp` )

In my Ubuntu VM this got me GIMP 2.10 so I will refer to it by version number later.
You may have to change the version number for your commands.

Install some `Qt5` dependencies for `texconv`
( `sudo apt install qt5-qmake qtbase5-dev` )


**Repo contents**
Whatever the directory you cloned this github repo to is named and wherever it is located, it will be referred to in this document as

`doom64-dc`

This guide will assume that you cloned it into your home directory. 

If you need to get to the top level of the repo, it will say

    cd ~/doom64-dc

Under doom64-dc, you will find the following files and directories

    doom64-dc/
    -- README.md (you're reading it right now)
    -- d64dcprep/
    ---- gimpscripts/
    ------ batch-doom64-colorconvert-nodither.scm (GIMP script for recoloring indexed-color images without dithering)
    ------ batch-doom64-colorconvert.scm (GIMP script for recoloring indexed-color images with dithering)
    ------ doom64-alt-monster.scm (GIMP script for performing a palette remap on indexed-color images)
    ------ doom64-tile-crop.scm (GIMP script for resizing and flattening indexed-color images)
    ------ morph-filename.scm (GIMP script for replacing the extension on a filename)
    ---- json/
    ---- monster/
    ------ tmp/
    -------- a (placeholder file for non-empty directory)
    ---- nonenemy/
    ------ tmp/
    -------- a (placeholder file for non-empty directory)
    ---- palgpl/
    ------ doom64monster.gpl  (GIMP format palette for recoloring monster sprites)
    ------ doom64nonenemy.gpl (GIMP format palette for recoloring nonenemy sprites)
    ---- palpngs/
    ------ palbaro.png (indexed color PNG with specific palette needed to remap BOSS to BARO)
    ------ palnite.png (indexed color PNG with specific palette needed to remap TROO to NITE) 
    ------ palply1.png (indexed color PNG with specific palette needed to remap green PLAY to red PLAY)
    ------ palply2.png (indexed color PNG with specific palette needed to remap green PLAY to blue PLAY)
    ------ palspec.png (indexed color PNG with specific palette needed to remap SARG to SPEC)
    ------ palzomb.png (indexed color PNG with specific palette needed to remap POSS to Immorpher's "blue" ZOMB)
    ---- sheets/
    ------ a (placeholder file for non-empty directory)
    ---- texconv_doom64dc.patch
    ---- wadfiles/
    ------ a (placeholder file for non-empty directory)
    ---- wadtools/
    ------ encode.c (compress a file with Doom `jaguar`/`LZSS` compression)
    ------ fixwad.c (replace monster sprite lumps in `doom64.wad`)
    ------ makelump.c (create a `spriteN64_t` lump from a raw image file)
    ------ makewad.c (create a new WAD file with alternate palette monster sprites)
    ------ twiddle.c (twiddle the graphic data stored in a `spriteN64_t` lump file)
    -- selfboot/
    ---- ogg/
    ------ lower/ 
    -------- *.ogg (all of the music tracks as 44khz mono OGG)
    ---- sfx/
    ------ *.wav (all of the game sfx as 22khz ADPCM WAV)
    ---- vq/ (where the sprite sheet for non enemy sprites ends up)
    ------ a (placeholder file for non-empty directory)


How to generate sprite textures / updated WAD files:

A subdirectory exists in the repo to use for placing all the files needed to make a bootable disc image
referred to hereon out as `doom64-dc/selfboot` .

A subdirectory exists in the repo to use for all of the texture preparation
referred to as `doom64-dc/d64dcprep`

Check that your doom64 ROM is the correct one

The below is the expected md5sum output

    md5sum doom64.z64
    b67748b64a2cc7efd2f3ad4504561e0e doom64.z64


dump the original N64 Doom 64 IWAD from the ROM

    dd if=doom64.z64 of=~/doom64-dc/d64dcprep/wadfiles/doom64.wad bs=1 skip=408848 count=6101168

Run Doom64ex's WADGEN against `doom64.z64` to dump a PC version of `DOOM64.WAD`.

Put the PC wad somewhere else not in the repo directory tree, we don't want to mess things up.

Open the PC wad in SLADE.

From SLADE, select lumps 1 through 346,
right-click, click on Export
select the `~/doom64-dc/d64dcprep/nonenemy` directory and click Save

select lumps 906 through 947
right-click, click on Export
select the `~/doom64-dc/d64dcprep/nonenemy` directory and click Save

next, in SLADE, select lumps 347 through 905 
right-click, click on Export
select the `~/doom64-dc/d64dcprep/monster` directory and click Save

From here until the end of the guide, you can mostly copy/paste these commands as long as you are using the assumed repo clone location
of `~/doom64-dc`.

In a terminal

    cd ~/doom64-dc/d64dcprep
    cp palgpl/doom64*.gpl ~/.config/GIMP/2.10/palettes
    cp gimpscripts/*.scm ~/.config/GIMP/2.10/scripts
    cp palpngs/*.png monster/tmp
    cd monster
    cp BOSS*.png tmp
    rename 's/BOSS/BARO/' tmp/BOSS*.png
    cp POSS*.png tmp
    rename 's/POSS/ZOMB/' tmp/POSS*.png
    cp SARG*.png tmp
    rename 's/SARG/SPEC/' tmp/SARG*.png
    cp TROO*.png tmp
    rename 's/TROO/NITE/' tmp/TROO*.png
    cp PLAY*.png tmp
    rename 's/PLAY/PLY1/' tmp/PLAY*.png
    cp PLAY*.png tmp
    rename 's/PLAY/PLY2/' tmp/PLAY*.png
    cd tmp
    gimp --verbose -i -b '(doom64-alt-monster "BARO*.png" "palbaro.png")' -b '(gimp-quit 0)'
    gimp --verbose -i -b '(doom64-alt-monster "ZOMB*.png" "palzomb.png")' -b '(gimp-quit 0)'
    gimp --verbose -i -b '(doom64-alt-monster "SPEC*.png" "palspec.png")' -b '(gimp-quit 0)'
    gimp --verbose -i -b '(doom64-alt-monster "NITE*.png" "palnite.png")' -b '(gimp-quit 0)'
    gimp --verbose -i -b '(doom64-alt-monster "PLY1*.png" "palply1.png")' -b '(gimp-quit 0)'
    gimp --verbose -i -b '(doom64-alt-monster "PLY2*.png" "palply2.png")' -b '(gimp-quit 0)'
    mv BARO*.png ..
    mv NITE*.png ..
    mv SPEC*.png ..
    mv ZOMB*.png ..
    mv PLY1*.png ..
    mv PLY2*.png ..
    cd ..
    gimp --verbose -i -b '(batch-doom64-colorconvert "*.png" "doom64monster")' -b '(gimp-quit 0)' 
    cd ..
    cd nonenemy
    mv ARM*.png tmp
    gimp --verbose -i -b '(batch-doom64-colorconvert "*.png" "doom64nonenemy")' -b '(gimp-quit 0)'    
    cd tmp
    gimp --verbose -i -b '(batch-doom64-colorconvert-nodither "ARM*.png" "doom64nonenemy")' -b '(gimp-quit 0)'   
    mv ARM*.png ..   
    
now that the individual graphics have been reindexed and flattened, it is time to make a spritesheet texture out of the item/decoration sprites

    cd ~/doom64-dc/d64dcprep
    python3 sheeter.py

The output will be a single png file (`nonenemy.png`) in the `doom64-dc/d64dcprep/sheets` subdirectory

You now need to clone the `texconv` github repo, apply a patch to it
and build it.

    cd ~/doom64-dc
    git clone https://github.com/tvspelsfreak/texconv.git
    mv texconv_doom64dc.patch texconv
    cd texconv
    git apply texconv_doom64dc.patch
    qmake
    make

Next, add the `texconv` directory to your path to make the next steps easier, and run `texconv` to create a .tex format sprite sheet texture.

    export PATH="~/doom64-dc/texconv:$PATH"
    cd ~/doom64dc/d64dcprep/sheets
    texconv --in non_enemy.png --out non_enemy.tex --format PAL8BPP --nonenemy
    rm *.tex.pal
    mv *.tex ~/doom64-dc/selfboot/vq

Now that the items/weapons/decorations have been handled, it is time to finish preparing the monster graphics.
Scripts will be run that will be resize the graphics if necessary, write them out as RAW files, further process them into `spriteN64_t` lumps, twiddle the graphic data for the PVR and `jaguar` compress the new graphics lumps for inclusion in the WAD file(s).

This step is probably the most time-consuming. You can literally paste the whole thing into the terminal and hit enter, it'll take
an hour to 90 minutes or so depending on your CPU and disk speed.

    cd ~/doom64-dc/d64dcprep/monster
    gimp -i -b '(doom64-tile-crop "SARGA1.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGB1.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGC1.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGD1.png" "doom64monster" 64 80 0 64 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGE1.png" "doom64monster" 80 82 0 80 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGF1.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGG1.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGH1.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGA2A8.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGB2B8.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGC2C8.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGD2D8.png" "doom64monster" 64 80 0 64 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGE2E8.png" "doom64monster" 88 82 0 88 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGF2F8.png" "doom64monster" 88 84 0 88 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGG2G8.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGH2H8.png" "doom64monster" 56 93 0 56 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGA3A7.png" "doom64monster" 80 79 0 80 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGB3B7.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGC3C7.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGD3D7.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGE3E7.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGF3F7.png" "doom64monster" 80 83 0 80 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGG3G7.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGH3H7.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGA4A6.png" "doom64monster" 72 80 0 72 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGB4B6.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGC4C6.png" "doom64monster" 72 83 0 72 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGD4D6.png" "doom64monster" 64 75 0 64 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGE4E6.png" "doom64monster" 72 78 0 72 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGF4F6.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGG4G6.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGH4H6.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGA5.png" "doom64monster" 64 76 0 64 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGB5.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGC5.png" "doom64monster" 64 78 0 64 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGD5.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGE5.png" "doom64monster" 80 77 0 80 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGF5.png" "doom64monster" 80 79 0 80 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGG5.png" "doom64monster" 72 76 0 72 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGH5.png" "doom64monster" 64 83 0 64 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGI0.png" "doom64monster" 80 99 0 80 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGJ0.png" "doom64monster" 80 100 0 80 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGK0.png" "doom64monster" 72 71 0 72 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGL0.png" "doom64monster" 72 75 0 72 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGM0.png" "doom64monster" 72 64 0 72 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SARGN0.png" "doom64monster" 64 42 0 64 42 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYA1.png" "doom64monster" 32 76 0 32 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYB1.png" "doom64monster" 32 77 0 32 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYC1.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYD1.png" "doom64monster" 40 78 0 40 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYE1.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYF1.png" "doom64monster" 32 80 0 32 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYG1.png" "doom64monster" 32 74 0 32 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYA2A8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYB2B8.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYC2C8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYD2D8.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYE2E8.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYF2F8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYG2G8.png" "doom64monster" 48 73 0 48 73 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYA3A7.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYB3B7.png" "doom64monster" 72 83 0 72 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYC3C7.png" "doom64monster" 56 81 0 56 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYD3D7.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYE3E7.png" "doom64monster" 64 85 0 64 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYF3F7.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYG3G7.png" "doom64monster" 48 79 0 48 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYA4A6.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYB4B6.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYC4C6.png" "doom64monster" 48 76 0 48 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYD4D6.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYE4E6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYF4F6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYG4G6.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYA5.png" "doom64monster" 32 79 0 32 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYB5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYC5.png" "doom64monster" 40 80 0 40 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYD5.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYE5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYF5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYG5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYH0.png" "doom64monster" 40 63 0 40 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYI0.png" "doom64monster" 48 66 0 48 66 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYJ0.png" "doom64monster" 48 65 0 48 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYK0.png" "doom64monster" 48 57 0 48 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYL0.png" "doom64monster" 48 43 0 48 43 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYM0.png" "doom64monster" 56 108 0 56 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYN0.png" "doom64monster" 48 62 0 48 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYO0.png" "doom64monster" 48 63 0 48 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYP0.png" "doom64monster" 56 61 0 56 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYQ0.png" "doom64monster" 56 57 0 56 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYR0.png" "doom64monster" 56 49 0 56 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYS0.png" "doom64monster" 56 44 0 56 44 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYT0.png" "doom64monster" 56 31 0 56 31 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYU0.png" "doom64monster" 56 24 0 56 24 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLAYV0.png" "doom64monster" 56 21 0 56 21 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOA1.png" "doom64monster" 48 91 0 48 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOB1.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOC1.png" "doom64monster" 48 90 0 48 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOD1.png" "doom64monster" 48 87 0 48 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOE1.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOF1.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOG1.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOH1.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOI1.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOJ1.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOK1.png" "doom64monster" 40 82 0 40 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOA2A8.png" "doom64monster" 48 91 0 48 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOB2B8.png" "doom64monster" 56 88 0 56 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOC2C8.png" "doom64monster" 40 91 0 40 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOD2D8.png" "doom64monster" 56 86 0 56 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOE2E8.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOF2F8.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOG2G8.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOH2H8.png" "doom64monster" 56 90 0 56 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOI2I8.png" "doom64monster" 64 83 0 64 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOJ2J8.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOK2K8.png" "doom64monster" 72 77 0 72 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOA3A7.png" "doom64monster" 48 87 0 48 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOB3B7.png" "doom64monster" 64 85 0 64 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOC3C7.png" "doom64monster" 48 90 0 48 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOD3D7.png" "doom64monster" 72 81 0 72 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOE3E7.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOF3F7.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOG3G7.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOH3H7.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOI3I7.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOJ3J7.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOK3K7.png" "doom64monster" 72 79 0 72 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOA4A6.png" "doom64monster" 40 85 0 40 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOB4B6.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOC4C6.png" "doom64monster" 48 88 0 48 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOD4D6.png" "doom64monster" 56 86 0 56 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOE4E6.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOF4F6.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOG4G6.png" "doom64monster" 48 82 0 48 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOH4H6.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOI4I6.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOJ4J6.png" "doom64monster" 48 88 0 48 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOK4K6.png" "doom64monster" 56 79 0 56 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOA5.png" "doom64monster" 48 85 0 48 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOB5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOC5.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOD5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOE5.png" "doom64monster" 72 87 0 72 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOF5.png" "doom64monster" 64 86 0 64 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOG5.png" "doom64monster" 48 79 0 48 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOH5.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOI5.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOJ5.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOK5.png" "doom64monster" 40 76 0 40 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOL0.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOM0.png" "doom64monster" 56 70 0 56 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROON0.png" "doom64monster" 64 70 0 64 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOO0.png" "doom64monster" 56 56 0 56 56 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOP0.png" "doom64monster" 56 36 0 56 36 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOQ0.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOR0.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOS0.png" "doom64monster" 88 91 0 88 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOT0.png" "doom64monster" 88 74 0 88 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOU0.png" "doom64monster" 80 69 0 80 69 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOV0.png" "doom64monster" 80 48 0 80 48 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOW0.png" "doom64monster" 72 34 0 72 34 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "TROOX0.png" "doom64monster" 72 21 0 72 21 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSA1.png" "doom64monster" 64 101 0 64 101 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSA5.png" "doom64monster" 64 93 0 64 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSA4A6.png" "doom64monster" 56 97 0 56 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSA3A7.png" "doom64monster" 40 103 0 40 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSA2A8.png" "doom64monster" 56 103 0 56 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSB1.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSB5.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSB4B6.png" "doom64monster" 56 93 0 56 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSB3B7.png" "doom64monster" 64 97 0 64 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSB2B8.png" "doom64monster" 64 100 0 64 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSC1.png" "doom64monster" 56 99 0 56 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSC5.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSC4C6.png" "doom64monster" 48 97 0 48 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSC3C7.png" "doom64monster" 48 100 0 48 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSC2C8.png" "doom64monster" 56 103 0 56 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSD1.png" "doom64monster" 56 97 0 56 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSD5.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSD4D6.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSD3D7.png" "doom64monster" 72 96 0 72 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSD2D8.png" "doom64monster" 64 99 0 64 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSE1.png" "doom64monster" 72 95 0 72 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSE5.png" "doom64monster" 64 110 0 64 110 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSE4E6.png" "doom64monster" 48 109 0 48 109 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSE3E7.png" "doom64monster" 80 103 0 80 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSE2E8.png" "doom64monster" 80 99 0 80 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSF1.png" "doom64monster" 72 97 0 72 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSF5.png" "doom64monster" 72 90 0 72 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSF4F6.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSF3F7.png" "doom64monster" 64 96 0 64 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSF2F8.png" "doom64monster" 72 94 0 72 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSG1.png" "doom64monster" 48 95 0 48 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSG5.png" "doom64monster" 56 83 0 56 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSG4G6.png" "doom64monster" 72 88 0 72 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSG3G7.png" "doom64monster" 72 92 0 72 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSG2G8.png" "doom64monster" 72 92 0 72 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSH1.png" "doom64monster" 64 94 0 64 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSH5.png" "doom64monster" 64 91 0 64 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSH4H6.png" "doom64monster" 72 97 0 72 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSH3H7.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSH2H8.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSI0.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSJ0.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSK0.png" "doom64monster" 64 74 0 64 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSL0.png" "doom64monster" 56 61 0 56 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSM0.png" "doom64monster" 56 48 0 56 48 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BOSSN0.png" "doom64monster" 64 34 0 64 34 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTA1.png" "doom64monster" 112 104 0 112 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTB1.png" "doom64monster" 112 105 0 112 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTC1.png" "doom64monster" 120 104 0 120 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTD1.png" "doom64monster" 112 104 0 112 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTE1.png" "doom64monster" 112 103 0 112 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTF1.png" "doom64monster" 120 103 0 120 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTG1.png" "doom64monster" 112 106 0 112 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTH1.png" "doom64monster" 112 108 0 112 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTI1.png" "doom64monster" 112 108 0 112 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTA2A8.png" "doom64monster" 88 105 0 88 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTB2B8.png" "doom64monster" 112 109 0 112 109 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTC2C8.png" "doom64monster" 120 107 0 120 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTD2D8.png" "doom64monster" 112 106 0 112 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTE2E8.png" "doom64monster" 104 105 0 104 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTF2F8.png" "doom64monster" 96 105 0 96 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTG2G8.png" "doom64monster" 104 110 0 104 110 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTH2H8.png" "doom64monster" 104 111 0 104 111 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTI2I8.png" "doom64monster" 88 108 0 88 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTA3A7.png" "doom64monster" 96 107 0 96 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTB3B7.png" "doom64monster" 80 111 0 80 111 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTC3C7.png" "doom64monster" 104 111 0 104 111 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTD3D7.png" "doom64monster" 104 107 0 104 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTE3E7.png" "doom64monster" 96 106 0 96 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTF3F7.png" "doom64monster" 104 104 0 104 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTG3G7.png" "doom64monster" 104 109 0 104 109 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTH3H7.png" "doom64monster" 72 110 0 72 110 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTI3I7.png" "doom64monster" 104 108 0 104 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTA4A6.png" "doom64monster" 96 104 0 96 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTB4B6.png" "doom64monster" 96 106 0 96 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTC4C6.png" "doom64monster" 104 107 0 104 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTD4D6.png" "doom64monster" 96 105 0 96 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTE4E6.png" "doom64monster" 96 104 0 96 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTF4F6.png" "doom64monster" 104 102 0 104 102 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTG4G6.png" "doom64monster" 120 104 0 120 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTH4H6.png" "doom64monster" 104 107 0 104 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTI4I6.png" "doom64monster" 104 106 0 104 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTA5.png" "doom64monster" 96 105 0 96 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTB5.png" "doom64monster" 96 106 0 96 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTC5.png" "doom64monster" 112 104 0 112 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTD5.png" "doom64monster" 104 104 0 104 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTE5.png" "doom64monster" 96 103 0 96 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTF5.png" "doom64monster" 104 103 0 104 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTG5.png" "doom64monster" 96 104 0 96 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTH5.png" "doom64monster" 112 106 0 112 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTI5.png" "doom64monster" 96 105 0 96 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTJ0.png" "doom64monster" 112 105 0 112 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTK0.png" "doom64monster" 112 96 0 112 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTL0.png" "doom64monster" 120 80 0 120 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTM0.png" "doom64monster" 120 79 0 120 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTN0.png" "doom64monster" 120 74 0 120 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "FATTO0.png" "doom64monster" 120 70 0 120 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULA1.png" "doom64monster" 56 62 0 56 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULB1.png" "doom64monster" 56 63 0 56 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULC1.png" "doom64monster" 56 58 0 56 58 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULD1.png" "doom64monster" 56 64 0 56 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULE1.png" "doom64monster" 56 66 0 56 66 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULF1.png" "doom64monster" 56 59 0 56 59 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULA2A8.png" "doom64monster" 48 61 0 48 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULB2B8.png" "doom64monster" 48 62 0 48 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULC2C8.png" "doom64monster" 48 68 0 48 68 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULD2D8.png" "doom64monster" 48 64 0 48 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULE2E8.png" "doom64monster" 56 69 0 56 69 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULF2F8.png" "doom64monster" 48 63 0 48 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULA3A7.png" "doom64monster" 48 72 0 48 72 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULB3B7.png" "doom64monster" 48 71 0 48 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULC3C7.png" "doom64monster" 48 71 0 48 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULD3D7.png" "doom64monster" 48 76 0 48 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULE3E7.png" "doom64monster" 48 71 0 48 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULF3F7.png" "doom64monster" 48 67 0 48 67 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULA4A6.png" "doom64monster" 56 59 0 56 59 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULB4B6.png" "doom64monster" 56 63 0 56 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULC4C6.png" "doom64monster" 56 64 0 56 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULD4D6.png" "doom64monster" 64 72 0 64 72 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULE4E6.png" "doom64monster" 56 79 0 56 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULF4F6.png" "doom64monster" 48 61 0 48 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULA5.png" "doom64monster" 56 59 0 56 59 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULB5.png" "doom64monster" 56 62 0 56 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULC5.png" "doom64monster" 56 63 0 56 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULD5.png" "doom64monster" 56 68 0 56 68 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULE5.png" "doom64monster" 56 70 0 56 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULF5.png" "doom64monster" 48 58 0 48 58 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULG0.png" "doom64monster" 56 59 0 56 59 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULH0.png" "doom64monster" 72 74 0 72 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULI0.png" "doom64monster" 72 72 0 72 72 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULJ0.png" "doom64monster" 72 68 0 72 68 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULK0.png" "doom64monster" 72 66 0 72 66 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULL0.png" "doom64monster" 64 65 0 64 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULM0.png" "doom64monster" 72 57 0 72 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULN0.png" "doom64monster" 72 60 0 72 60 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULO0.png" "doom64monster" 64 49 0 64 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SKULP0.png" "doom64monster" 56 47 0 56 47 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINA1.png" "doom64monster" 112 102 0 112 102 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINB1.png" "doom64monster" 112 92 0 112 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINC1.png" "doom64monster" 112 90 0 112 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAIND1.png" "doom64monster" 112 99 0 112 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINA2A8.png" "doom64monster" 96 97 0 96 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINB2B8.png" "doom64monster" 96 97 0 96 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINC2C8.png" "doom64monster" 96 91 0 96 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAIND2D8.png" "doom64monster" 96 103 0 96 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINA3A7.png" "doom64monster" 104 102 0 104 102 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINB3B7.png" "doom64monster" 104 96 0 104 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINC3C7.png" "doom64monster" 104 90 0 104 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAIND3D7.png" "doom64monster" 104 105 0 104 105 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINA4A6.png" "doom64monster" 104 102 0 104 102 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINB4B6.png" "doom64monster" 112 98 0 112 98 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINC4C6.png" "doom64monster" 112 93 0 112 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAIND4D6.png" "doom64monster" 104 104 0 104 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINA5.png" "doom64monster" 104 107 0 104 107 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINB5.png" "doom64monster" 104 106 0 104 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINC5.png" "doom64monster" 112 106 0 112 106 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAIND5.png" "doom64monster" 104 100 0 104 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINE0.png" "doom64monster" 112 96 0 112 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINF0.png" "doom64monster" 120 98 0 120 98 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAING0.png" "doom64monster" 120 95 0 120 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINH0.png" "doom64monster" 112 95 0 112 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINI0.png" "doom64monster" 112 89 0 112 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINJ0.png" "doom64monster" 112 89 0 112 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINK0.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PAINL0.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIA1.png" "doom64monster" 144 84 0 144 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIB1.png" "doom64monster" 136 79 0 136 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIC1.png" "doom64monster" 144 83 0 144 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPID1.png" "doom64monster" 136 80 0 136 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIE1.png" "doom64monster" 128 78 0 128 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIF1.png" "doom64monster" 128 85 0 128 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIA2A8.png" "doom64monster" 128 90 0 128 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIB2B8.png" "doom64monster" 128 86 0 128 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIC2C8.png" "doom64monster" 136 85 0 136 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPID2D8.png" "doom64monster" 120 85 0 120 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIE2E8.png" "doom64monster" 120 85 0 120 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIF2F8.png" "doom64monster" 120 94 0 120 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIA3A7.png" "doom64monster" 88 92 0 88 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIB3B7.png" "doom64monster" 96 89 0 96 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIC3C7.png" "doom64monster" 88 89 0 88 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPID3D7.png" "doom64monster" 88 87 0 88 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIE3E7.png" "doom64monster" 104 84 0 104 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIF3F7.png" "doom64monster" 88 99 0 88 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIA4A6.png" "doom64monster" 136 97 0 136 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIB4B6.png" "doom64monster" 120 95 0 120 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIC4C6.png" "doom64monster" 128 91 0 128 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPID4D6.png" "doom64monster" 136 89 0 136 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIE4E6.png" "doom64monster" 128 89 0 128 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIF4F6.png" "doom64monster" 120 103 0 120 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIA5.png" "doom64monster" 152 89 0 152 89 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIB5.png" "doom64monster" 136 88 0 136 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIC5.png" "doom64monster" 152 88 0 152 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPID5.png" "doom64monster" 136 88 0 136 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIE5.png" "doom64monster" 128 83 0 128 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIF5.png" "doom64monster" 128 95 0 128 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIG0.png" "doom64monster" 128 84 0 128 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIH0.png" "doom64monster" 128 75 0 128 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPII0.png" "doom64monster" 128 69 0 128 69 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIJ0.png" "doom64monster" 128 59 0 128 59 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIK0.png" "doom64monster" 128 52 0 128 52 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BSPIL0.png" "doom64monster" 120 51 0 120 51 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSA1.png" "doom64monster" 40 80 0 40 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSB1.png" "doom64monster" 40 82 0 40 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSC1.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSD1.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSE1.png" "doom64monster" 40 77 0 40 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSF1.png" "doom64monster" 32 77 0 32 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSG1.png" "doom64monster" 48 71 0 48 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSA2A8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSB2B8.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSC2C8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSD2D8.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSE2E8.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSF2F8.png" "doom64monster" 56 79 0 56 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSG2G8.png" "doom64monster" 56 74 0 56 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSA3A7.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSB3B7.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSC3C7.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSD3D7.png" "doom64monster" 56 83 0 56 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSE3E7.png" "doom64monster" 64 75 0 64 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSF3F7.png" "doom64monster" 64 76 0 64 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSG3G7.png" "doom64monster" 48 74 0 48 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSA4A6.png" "doom64monster" 48 80 0 48 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSB4B6.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSC4C6.png" "doom64monster" 48 81 0 48 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSD4D6.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSE4E6.png" "doom64monster" 40 74 0 40 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSF4F6.png" "doom64monster" 48 75 0 48 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSG4G6.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSA5.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSB5.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSC5.png" "doom64monster" 40 77 0 40 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSD5.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSE5.png" "doom64monster" 40 72 0 40 72 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSF5.png" "doom64monster" 32 74 0 32 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSG5.png" "doom64monster" 48 78 0 48 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSH0.png" "doom64monster" 56 76 0 56 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSI0.png" "doom64monster" 56 65 0 56 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSJ0.png" "doom64monster" 48 49 0 48 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSK0.png" "doom64monster" 48 34 0 48 34 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSL0.png" "doom64monster" 48 27 0 48 27 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSM0.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSN0.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSO0.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSP0.png" "doom64monster" 64 64 0 64 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSQ0.png" "doom64monster" 64 53 0 64 53 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSR0.png" "doom64monster" 64 46 0 64 46 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSS0.png" "doom64monster" 64 36 0 64 36 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSST0.png" "doom64monster" 64 24 0 64 24 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "POSSU0.png" "doom64monster" 72 19 0 72 19 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADA1.png" "doom64monster" 112 95 0 112 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADB1.png" "doom64monster" 104 93 0 104 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADC1.png" "doom64monster" 112 98 0 112 98 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADD1.png" "doom64monster" 104 94 0 104 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADE1.png" "doom64monster" 112 90 0 112 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADF1.png" "doom64monster" 120 85 0 120 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADG1.png" "doom64monster" 120 76 0 120 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADA2A8.png" "doom64monster" 104 104 0 104 104 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADB2B8.png" "doom64monster" 88 100 0 88 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADC2C8.png" "doom64monster" 72 102 0 72 102 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADD2D8.png" "doom64monster" 88 101 0 88 101 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADE2E8.png" "doom64monster" 88 90 0 88 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADF2F8.png" "doom64monster" 96 85 0 96 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADG2G8.png" "doom64monster" 96 80 0 96 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADA3A7.png" "doom64monster" 64 108 0 64 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADB3B7.png" "doom64monster" 64 103 0 64 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADC3C7.png" "doom64monster" 72 97 0 72 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADD3D7.png" "doom64monster" 64 99 0 64 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADE3E7.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADF3F7.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADG3G7.png" "doom64monster" 72 74 0 72 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADA4A6.png" "doom64monster" 72 90 0 72 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADB4B6.png" "doom64monster" 88 93 0 88 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADC4C6.png" "doom64monster" 96 88 0 96 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADD4D6.png" "doom64monster" 88 91 0 88 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADE4E6.png" "doom64monster" 88 83 0 88 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADF4F6.png" "doom64monster" 88 77 0 88 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADG4G6.png" "doom64monster" 96 71 0 96 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADA5.png" "doom64monster" 96 92 0 96 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADB5.png" "doom64monster" 104 84 0 104 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADC5.png" "doom64monster" 96 91 0 96 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADD5.png" "doom64monster" 104 83 0 104 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADE5.png" "doom64monster" 96 81 0 96 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADF5.png" "doom64monster" 88 75 0 88 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADG5.png" "doom64monster" 96 65 0 96 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADH0.png" "doom64monster" 120 98 0 120 98 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADI0.png" "doom64monster" 112 84 0 112 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADJ0.png" "doom64monster" 112 74 0 112 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADK0.png" "doom64monster" 104 67 0 104 67 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADL0.png" "doom64monster" 88 60 0 88 60 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "HEADM0.png" "doom64monster" 88 55 0 88 55 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA1.png" "doom64nonenemy" 120 163 0 120 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB1.png" "doom64nonenemy" 112 165 0 112 165 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC1.png" "doom64nonenemy" 104 164 0 104 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD1.png" "doom64nonenemy" 120 166 0 120 166 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE1.png" "doom64nonenemy" 104 167 0 104 167 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF1.png" "doom64nonenemy" 112 166 0 112 166 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA8.png" "doom64nonenemy" 88 162 0 88 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB8.png" "doom64nonenemy" 96 164 0 96 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC8.png" "doom64nonenemy" 104 166 0 104 166 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD8.png" "doom64nonenemy" 96 167 0 96 167 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE8.png" "doom64nonenemy" 144 168 0 144 168 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF8.png" "doom64nonenemy" 128 168 0 128 168 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA7.png" "doom64nonenemy" 88 162 0 88 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB7.png" "doom64nonenemy" 64 163 0 64 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC7.png" "doom64nonenemy" 88 162 0 88 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD7.png" "doom64nonenemy" 64 168 0 64 168 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE7.png" "doom64nonenemy" 128 164 0 128 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF7.png" "doom64nonenemy" 104 164 0 104 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA6.png" "doom64nonenemy" 104 163 0 104 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB6.png" "doom64nonenemy" 88 164 0 88 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC6.png" "doom64nonenemy" 96 158 0 96 158 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD6.png" "doom64nonenemy" 96 165 0 96 165 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE6.png" "doom64nonenemy" 96 158 0 96 158 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF6.png" "doom64nonenemy" 96 157 0 96 157 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA5.png" "doom64nonenemy" 120 158 0 120 158 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB5.png" "doom64nonenemy" 112 162 0 112 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC5.png" "doom64nonenemy" 112 157 0 112 157 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD5.png" "doom64nonenemy" 112 160 0 112 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE5.png" "doom64nonenemy" 104 159 0 104 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF5.png" "doom64nonenemy" 112 159 0 112 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA4.png" "doom64nonenemy" 88 162 0 88 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB4.png" "doom64nonenemy" 104 168 0 104 168 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC4.png" "doom64nonenemy" 120 163 0 120 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD4.png" "doom64nonenemy" 96 159 0 96 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE4.png" "doom64nonenemy" 128 170 0 128 170 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF4.png" "doom64nonenemy" 112 170 0 112 170 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA3.png" "doom64nonenemy" 96 163 0 96 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB3.png" "doom64nonenemy" 72 167 0 72 167 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC3.png" "doom64nonenemy" 96 164 0 96 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD3.png" "doom64nonenemy" 72 159 0 72 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE3.png" "doom64nonenemy" 152 170 0 152 170 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF3.png" "doom64nonenemy" 120 170 0 120 170 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRA2.png" "doom64nonenemy" 112 167 0 112 167 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRB2.png" "doom64nonenemy" 104 170 0 104 170 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRC2.png" "doom64nonenemy" 104 166 0 104 166 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRD2.png" "doom64nonenemy" 104 162 0 104 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRE2.png" "doom64nonenemy" 96 163 0 96 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRF2.png" "doom64nonenemy" 96 163 0 96 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRG0.png" "doom64nonenemy" 88 167 0 88 167 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRH0.png" "doom64nonenemy" 96 162 0 96 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRI0.png" "doom64nonenemy" 96 159 0 96 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRJ0.png" "doom64nonenemy" 112 156 0 112 156 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRK0.png" "doom64nonenemy" 112 152 0 112 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRL0.png" "doom64nonenemy" 112 153 0 112 153 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRM0.png" "doom64nonenemy" 112 152 0 112 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRN0.png" "doom64nonenemy" 104 142 0 104 142 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "CYBRO0.png" "doom64nonenemy" 104 30 0 104 30 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTA1.png" "doom64monster" 152 144 0 152 144 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTB1.png" "doom64monster" 144 143 0 144 143 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTC1.png" "doom64monster" 144 142 0 144 142 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTD1.png" "doom64monster" 144 142 0 144 142 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTE1.png" "doom64monster" 144 141 0 144 141 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTF1.png" "doom64monster" 160 149 0 160 149 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTG1.png" "doom64monster" 96 150 0 96 150 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTH1.png" "doom64monster" 144 156 0 144 156 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTA2A8.png" "doom64monster" 112 145 0 112 145 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTB2B8.png" "doom64monster" 112 145 0 112 145 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTC2C8.png" "doom64monster" 128 144 0 128 144 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTD2D8.png" "doom64monster" 120 144 0 120 144 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTE2E8.png" "doom64monster" 160 143 0 160 143 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTF2F8.png" "doom64monster" 144 163 0 144 163 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTG2G8.png" "doom64monster" 112 150 0 112 150 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTH2H8.png" "doom64monster" 112 152 0 112 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTA3A7.png" "doom64monster" 136 152 0 136 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTB3B7.png" "doom64monster" 136 151 0 136 151 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTC3C7.png" "doom64monster" 136 151 0 136 151 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTD3D7.png" "doom64monster" 136 151 0 136 151 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTE3E7.png" "doom64monster" 136 150 0 136 150 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTF3F7.png" "doom64monster" 136 161 0 136 161 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTG3G7.png" "doom64monster" 128 152 0 128 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTH3H7.png" "doom64monster" 136 152 0 136 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTA4A6.png" "doom64monster" 136 160 0 136 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTB4B6.png" "doom64monster" 136 160 0 136 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTC4C6.png" "doom64monster" 128 160 0 128 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTD4D6.png" "doom64monster" 120 159 0 120 159 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTE4E6.png" "doom64monster" 136 160 0 136 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTF4F6.png" "doom64monster" 128 164 0 128 164 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTG4G6.png" "doom64monster" 120 158 0 120 158 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTH4H6.png" "doom64monster" 128 160 0 128 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTA5.png" "doom64monster" 128 160 0 128 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTB5.png" "doom64monster" 120 160 0 120 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTC5.png" "doom64monster" 128 161 0 128 161 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTD5.png" "doom64monster" 120 160 0 120 160 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTE5.png" "doom64monster" 144 158 0 144 158 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTF5.png" "doom64monster" 160 162 0 160 162 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTG5.png" "doom64monster" 88 161 0 88 161 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTH5.png" "doom64monster" 136 161 0 136 161 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTI0.png" "doom64monster" 144 156 0 144 156 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTJ0.png" "doom64monster" 152 152 0 152 152 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTK0.png" "doom64monster" 168 153 0 168 153 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTL0.png" "doom64monster" 176 150 0 176 150 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTM0.png" "doom64monster" 176 156 0 176 156 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTN0.png" "doom64monster" 176 153 0 176 153 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "RECTO0.png" "doom64monster" 176 148 0 176 148 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1A1.png" "doom64monster" 32 76 0 32 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1B1.png" "doom64monster" 32 77 0 32 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1C1.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1D1.png" "doom64monster" 40 78 0 40 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1E1.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1F1.png" "doom64monster" 32 80 0 32 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1G1.png" "doom64monster" 32 74 0 32 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1A2A8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1B2B8.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1C2C8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1D2D8.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1E2E8.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1F2F8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1G2G8.png" "doom64monster" 48 73 0 48 73 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1A3A7.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1B3B7.png" "doom64monster" 72 83 0 72 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1C3C7.png" "doom64monster" 56 81 0 56 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1D3D7.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1E3E7.png" "doom64monster" 64 85 0 64 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1F3F7.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1G3G7.png" "doom64monster" 48 79 0 48 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1A4A6.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1B4B6.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1C4C6.png" "doom64monster" 48 76 0 48 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1D4D6.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1E4E6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1F4F6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1G4G6.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1A5.png" "doom64monster" 32 79 0 32 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1B5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1C5.png" "doom64monster" 40 80 0 40 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1D5.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1E5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1F5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1G5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1H0.png" "doom64monster" 40 63 0 40 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1I0.png" "doom64monster" 48 66 0 48 66 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1J0.png" "doom64monster" 48 65 0 48 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1K0.png" "doom64monster" 48 57 0 48 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1L0.png" "doom64monster" 48 43 0 48 43 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1M0.png" "doom64monster" 56 108 0 56 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1N0.png" "doom64monster" 48 62 0 48 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1O0.png" "doom64monster" 48 63 0 48 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1P0.png" "doom64monster" 56 61 0 56 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1Q0.png" "doom64monster" 56 57 0 56 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1R0.png" "doom64monster" 56 49 0 56 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1S0.png" "doom64monster" 56 44 0 56 44 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1T0.png" "doom64monster" 56 31 0 56 31 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1U0.png" "doom64monster" 56 24 0 56 24 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY1V0.png" "doom64monster" 56 21 0 56 21 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2A1.png" "doom64monster" 32 76 0 32 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2B1.png" "doom64monster" 32 77 0 32 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2C1.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2D1.png" "doom64monster" 40 78 0 40 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2E1.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2F1.png" "doom64monster" 32 80 0 32 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2G1.png" "doom64monster" 32 74 0 32 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2A2A8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2B2B8.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2C2C8.png" "doom64monster" 48 84 0 48 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2D2D8.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2E2E8.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2F2F8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2G2G8.png" "doom64monster" 48 73 0 48 73 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2A3A7.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2B3B7.png" "doom64monster" 72 83 0 72 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2C3C7.png" "doom64monster" 56 81 0 56 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2D3D7.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2E3E7.png" "doom64monster" 64 85 0 64 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2F3F7.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2G3G7.png" "doom64monster" 48 79 0 48 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2A4A6.png" "doom64monster" 40 75 0 40 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2B4B6.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2C4C6.png" "doom64monster" 48 76 0 48 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2D4D6.png" "doom64monster" 56 80 0 56 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2E4E6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2F4F6.png" "doom64monster" 48 83 0 48 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2G4G6.png" "doom64monster" 32 81 0 32 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2A5.png" "doom64monster" 32 79 0 32 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2B5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2C5.png" "doom64monster" 40 80 0 40 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2D5.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2E5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2F5.png" "doom64monster" 32 83 0 32 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2G5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2H0.png" "doom64monster" 40 63 0 40 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2I0.png" "doom64monster" 48 66 0 48 66 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2J0.png" "doom64monster" 48 65 0 48 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2K0.png" "doom64monster" 48 57 0 48 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2L0.png" "doom64monster" 48 43 0 48 43 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2M0.png" "doom64monster" 56 108 0 56 108 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2N0.png" "doom64monster" 48 62 0 48 62 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2O0.png" "doom64monster" 48 63 0 48 63 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2P0.png" "doom64monster" 56 61 0 56 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2Q0.png" "doom64monster" 56 57 0 56 57 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2R0.png" "doom64monster" 56 49 0 56 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2S0.png" "doom64monster" 56 44 0 56 44 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2T0.png" "doom64monster" 56 31 0 56 31 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2U0.png" "doom64monster" 56 24 0 56 24 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "PLY2V0.png" "doom64monster" 56 21 0 56 21 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBA1.png" "doom64monster" 40 80 0 40 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBB1.png" "doom64monster" 40 82 0 40 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBC1.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBD1.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBE1.png" "doom64monster" 40 77 0 40 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBF1.png" "doom64monster" 32 77 0 32 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBG1.png" "doom64monster" 48 71 0 48 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBA2A8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBB2B8.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBC2C8.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBD2D8.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBE2E8.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBF2F8.png" "doom64monster" 56 79 0 56 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBG2G8.png" "doom64monster" 56 74 0 56 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBA3A7.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBB3B7.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBC3C7.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBD3D7.png" "doom64monster" 56 83 0 56 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBE3E7.png" "doom64monster" 64 75 0 64 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBF3F7.png" "doom64monster" 64 76 0 64 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBG3G7.png" "doom64monster" 48 74 0 48 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBA4A6.png" "doom64monster" 48 80 0 48 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBB4B6.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBC4C6.png" "doom64monster" 48 81 0 48 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBD4D6.png" "doom64monster" 40 84 0 40 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBE4E6.png" "doom64monster" 40 74 0 40 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBF4F6.png" "doom64monster" 48 75 0 48 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBG4G6.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBA5.png" "doom64monster" 40 79 0 40 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBB5.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBC5.png" "doom64monster" 40 77 0 40 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBD5.png" "doom64monster" 40 81 0 40 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBE5.png" "doom64monster" 40 72 0 40 72 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBF5.png" "doom64monster" 32 74 0 32 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBG5.png" "doom64monster" 48 78 0 48 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBH0.png" "doom64monster" 56 76 0 56 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBI0.png" "doom64monster" 56 65 0 56 65 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBJ0.png" "doom64monster" 48 49 0 48 49 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBK0.png" "doom64monster" 48 34 0 48 34 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBL0.png" "doom64monster" 48 27 0 48 27 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBM0.png" "doom64monster" 56 78 0 56 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBN0.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBO0.png" "doom64monster" 64 81 0 64 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBP0.png" "doom64monster" 64 64 0 64 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBQ0.png" "doom64monster" 64 53 0 64 53 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBR0.png" "doom64monster" 64 46 0 64 46 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBS0.png" "doom64monster" 64 36 0 64 36 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBT0.png" "doom64monster" 64 24 0 64 24 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "ZOMBU0.png" "doom64monster" 72 19 0 72 19 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECA1.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECB1.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECC1.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECD1.png" "doom64monster" 64 80 0 64 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECE1.png" "doom64monster" 80 82 0 80 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECF1.png" "doom64monster" 80 84 0 80 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECG1.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECH1.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECA2A8.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECB2B8.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECC2C8.png" "doom64monster" 72 85 0 72 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECD2D8.png" "doom64monster" 64 80 0 64 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECE2E8.png" "doom64monster" 88 82 0 88 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECF2F8.png" "doom64monster" 88 84 0 88 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECG2G8.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECH2H8.png" "doom64monster" 56 93 0 56 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECA3A7.png" "doom64monster" 80 79 0 80 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECB3B7.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECC3C7.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECD3D7.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECE3E7.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECF3F7.png" "doom64monster" 80 83 0 80 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECG3G7.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECH3H7.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECA4A6.png" "doom64monster" 72 80 0 72 80 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECB4B6.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECC4C6.png" "doom64monster" 72 83 0 72 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECD4D6.png" "doom64monster" 64 75 0 64 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECE4E6.png" "doom64monster" 72 78 0 72 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECF4F6.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECG4G6.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECH4H6.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECA5.png" "doom64monster" 64 76 0 64 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECB5.png" "doom64monster" 64 79 0 64 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECC5.png" "doom64monster" 64 78 0 64 78 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECD5.png" "doom64monster" 64 77 0 64 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECE5.png" "doom64monster" 80 77 0 80 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECF5.png" "doom64monster" 80 79 0 80 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECG5.png" "doom64monster" 72 76 0 72 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECH5.png" "doom64monster" 64 83 0 64 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECI0.png" "doom64monster" 80 99 0 80 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECJ0.png" "doom64monster" 80 100 0 80 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECK0.png" "doom64monster" 72 71 0 72 71 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECL0.png" "doom64monster" 72 75 0 72 75 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECM0.png" "doom64monster" 72 64 0 72 64 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "SPECN0.png" "doom64monster" 64 42 0 64 42 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEA1.png" "doom64monster" 48 91 0 48 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEB1.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEC1.png" "doom64monster" 48 90 0 48 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITED1.png" "doom64monster" 48 87 0 48 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEE1.png" "doom64monster" 64 84 0 64 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEF1.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEG1.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEH1.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEI1.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEJ1.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEK1.png" "doom64monster" 40 82 0 40 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEA2A8.png" "doom64monster" 48 91 0 48 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEB2B8.png" "doom64monster" 56 88 0 56 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEC2C8.png" "doom64monster" 40 91 0 40 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITED2D8.png" "doom64monster" 56 86 0 56 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEE2E8.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEF2F8.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEG2G8.png" "doom64monster" 72 82 0 72 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEH2H8.png" "doom64monster" 56 90 0 56 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEI2I8.png" "doom64monster" 64 83 0 64 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEJ2J8.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEK2K8.png" "doom64monster" 72 77 0 72 77 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEA3A7.png" "doom64monster" 48 87 0 48 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEB3B7.png" "doom64monster" 64 85 0 64 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEC3C7.png" "doom64monster" 48 90 0 48 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITED3D7.png" "doom64monster" 72 81 0 72 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEE3E7.png" "doom64monster" 72 84 0 72 84 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEF3F7.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEG3G7.png" "doom64monster" 80 81 0 80 81 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEH3H7.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEI3I7.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEJ3J7.png" "doom64monster" 56 85 0 56 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEK3K7.png" "doom64monster" 72 79 0 72 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEA4A6.png" "doom64monster" 40 85 0 40 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEB4B6.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEC4C6.png" "doom64monster" 48 88 0 48 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITED4D6.png" "doom64monster" 56 86 0 56 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEE4E6.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEF4F6.png" "doom64monster" 64 87 0 64 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEG4G6.png" "doom64monster" 48 82 0 48 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEH4H6.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEI4I6.png" "doom64monster" 40 87 0 40 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEJ4J6.png" "doom64monster" 48 88 0 48 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEK4K6.png" "doom64monster" 56 79 0 56 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEA5.png" "doom64monster" 48 85 0 48 85 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEB5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEC5.png" "doom64monster" 48 86 0 48 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITED5.png" "doom64monster" 40 83 0 40 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEE5.png" "doom64monster" 72 87 0 72 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEF5.png" "doom64monster" 64 86 0 64 86 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEG5.png" "doom64monster" 48 79 0 48 79 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEH5.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEI5.png" "doom64monster" 56 87 0 56 87 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEJ5.png" "doom64monster" 56 82 0 56 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEK5.png" "doom64monster" 40 76 0 40 76 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEL0.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEM0.png" "doom64monster" 56 70 0 56 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEN0.png" "doom64monster" 64 70 0 64 70 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEO0.png" "doom64monster" 56 56 0 56 56 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEP0.png" "doom64monster" 56 36 0 56 36 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEQ0.png" "doom64monster" 64 92 0 64 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITER0.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITES0.png" "doom64monster" 88 91 0 88 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITET0.png" "doom64monster" 88 74 0 88 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEU0.png" "doom64monster" 80 69 0 80 69 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEV0.png" "doom64monster" 80 48 0 80 48 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEW0.png" "doom64monster" 72 34 0 72 34 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "NITEX0.png" "doom64monster" 72 21 0 72 21 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROA1.png" "doom64monster" 64 101 0 64 101 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROA5.png" "doom64monster" 64 93 0 64 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROA4A6.png" "doom64monster" 56 97 0 56 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROA3A7.png" "doom64monster" 40 103 0 40 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROA2A8.png" "doom64monster" 56 103 0 56 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROB1.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROB5.png" "doom64monster" 64 90 0 64 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROB4B6.png" "doom64monster" 56 93 0 56 93 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROB3B7.png" "doom64monster" 64 97 0 64 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROB2B8.png" "doom64monster" 64 100 0 64 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROC1.png" "doom64monster" 56 99 0 56 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROC5.png" "doom64monster" 56 94 0 56 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROC4C6.png" "doom64monster" 48 97 0 48 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROC3C7.png" "doom64monster" 48 100 0 48 100 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROC2C8.png" "doom64monster" 56 103 0 56 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROD1.png" "doom64monster" 56 97 0 56 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROD5.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROD4D6.png" "doom64monster" 56 92 0 56 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROD3D7.png" "doom64monster" 72 96 0 72 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROD2D8.png" "doom64monster" 64 99 0 64 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROE1.png" "doom64monster" 72 95 0 72 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROE5.png" "doom64monster" 64 110 0 64 110 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROE4E6.png" "doom64monster" 48 109 0 48 109 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROE3E7.png" "doom64monster" 80 103 0 80 103 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROE2E8.png" "doom64monster" 80 99 0 80 99 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROF1.png" "doom64monster" 72 97 0 72 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROF5.png" "doom64monster" 72 90 0 72 90 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROF4F6.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROF3F7.png" "doom64monster" 64 96 0 64 96 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROF2F8.png" "doom64monster" 72 94 0 72 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROG1.png" "doom64monster" 48 95 0 48 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROG5.png" "doom64monster" 56 83 0 56 83 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROG4G6.png" "doom64monster" 72 88 0 72 88 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROG3G7.png" "doom64monster" 72 92 0 72 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROG2G8.png" "doom64monster" 72 92 0 72 92 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROH1.png" "doom64monster" 64 94 0 64 94 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROH5.png" "doom64monster" 64 91 0 64 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROH4H6.png" "doom64monster" 72 97 0 72 97 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROH3H7.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROH2H8.png" "doom64monster" 64 95 0 64 95 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROI0.png" "doom64monster" 72 91 0 72 91 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROJ0.png" "doom64monster" 64 82 0 64 82 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROK0.png" "doom64monster" 64 74 0 64 74 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROL0.png" "doom64monster" 56 61 0 56 61 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BAROM0.png" "doom64monster" 56 48 0 56 48 0 0)' -b '(gimp-quit 0)'
    gimp -i -b '(doom64-tile-crop "BARON0.png" "doom64monster" 64 34 0 64 34 0 0)' -b '(gimp-quit 0)'

Once all of the gimp commands finish, you will have a raw file for each individual monster sprite.
These need to be converted into lumps with twiddled graphic data.

You need to build five small standalone C programs to complete the process of generating the Dreamcast Doom 64 data.
These are built with your host GCC compiler (for example `x86_64-linux-gnu-gcc`), not the Dreamcast `sh-elf-gcc`.

    cd ~/doom64-dc/d64dcprep/wadtools
	make

add the wadtools directory to your path so the tools can be found when you need to run them

    export PATH="~/doom64-dc/d64dcprep/wadtools:$PATH"

go back to the monster directory and run the `makelump` and `twiddle` tools against each sprite. Again you can copy paste this whole thing, it should only
take a few seconds to complete.

    cd ~/doom64-dc/d64dcprep/monster
    makelump SARGA1 62 84 31 84 5
    twiddle SARGA1 64 128
    makelump SARGB1 59 79 29 79 7
    twiddle SARGB1 64 128
    makelump SARGC1 59 82 29 82 9
    twiddle SARGC1 64 128
    makelump SARGD1 62 80 31 80 11
    twiddle SARGD1 64 128
    makelump SARGE1 77 82 39 82 13
    twiddle SARGE1 128 128
    makelump SARGF1 76 84 38 84 15
    twiddle SARGF1 128 128
    makelump SARGG1 68 84 35 84 17
    twiddle SARGG1 128 128
    makelump SARGH1 61 90 30 90 19
    twiddle SARGH1 64 128
    makelump SARGA2A8 67 84 34 83 21
    twiddle SARGA2A8 128 128
    makelump SARGB2B8 62 77 35 78 23
    twiddle SARGB2B8 64 128
    makelump SARGC2C8 72 85 36 85 25
    twiddle SARGC2C8 128 128
    makelump SARGD2D8 62 80 35 80 27
    twiddle SARGD2D8 64 128
    makelump SARGE2E8 81 82 44 82 29
    twiddle SARGE2E8 128 128
    makelump SARGF2F8 87 84 58 84 31
    twiddle SARGF2F8 128 128
    makelump SARGG2G8 72 84 43 84 33
    twiddle SARGG2G8 128 128
    makelump SARGH2H8 52 93 27 92 35
    twiddle SARGH2H8 64 128
    makelump SARGA3A7 73 79 36 79 37
    twiddle SARGA3A7 128 128
    makelump SARGB3B7 63 77 33 77 39
    twiddle SARGB3B7 64 128
    makelump SARGC3C7 73 81 35 81 41
    twiddle SARGC3C7 128 128
    makelump SARGD3D7 63 77 33 78 43
    twiddle SARGD3D7 64 128
    makelump SARGE3E7 70 82 36 82 45
    twiddle SARGE3E7 128 128
    makelump SARGF3F7 75 83 40 83 47
    twiddle SARGF3F7 128 128
    makelump SARGG3G7 72 84 37 84 49
    twiddle SARGG3G7 128 128
    makelump SARGH3H7 58 90 26 90 51
    twiddle SARGH3H7 64 128
    makelump SARGA4A6 70 80 35 80 53
    twiddle SARGA4A6 128 128
    makelump SARGB4B6 63 77 31 77 55
    twiddle SARGB4B6 64 128
    makelump SARGC4C6 69 83 34 83 57
    twiddle SARGC4C6 128 128
    makelump SARGD4D6 57 75 29 75 59
    twiddle SARGD4D6 64 128
    makelump SARGE4E6 71 78 45 78 61
    twiddle SARGE4E6 128 128
    makelump SARGF4F6 74 81 49 80 63
    twiddle SARGF4F6 128 128
    makelump SARGG4G6 63 79 38 78 65
    twiddle SARGG4G6 64 128
    makelump SARGH4H6 64 84 31 84 67
    twiddle SARGH4H6 64 128
    makelump SARGA5 63 76 29 76 69
    twiddle SARGA5 64 128
    makelump SARGB5 58 79 29 79 71
    twiddle SARGB5 64 128
    makelump SARGC5 61 78 33 78 73
    twiddle SARGC5 64 128
    makelump SARGD5 63 77 32 77 75
    twiddle SARGD5 64 128
    makelump SARGE5 75 77 37 77 77
    twiddle SARGE5 128 128
    makelump SARGF5 74 79 38 79 79
    twiddle SARGF5 128 128
    makelump SARGG5 70 76 34 76 81
    twiddle SARGG5 128 128
    makelump SARGH5 62 83 30 83 83
    twiddle SARGH5 64 128
    makelump SARGI0 76 99 41 99 85
    twiddle SARGI0 128 128
    makelump SARGJ0 75 100 39 100 87
    twiddle SARGJ0 128 128
    makelump SARGK0 66 71 34 71 89
    twiddle SARGK0 128 128
    makelump SARGL0 67 75 36 75 91
    twiddle SARGL0 128 128
    makelump SARGM0 66 64 35 64 93
    twiddle SARGM0 128 64
    makelump SARGN0 62 42 31 41 95
    twiddle SARGN0 64 64
    makelump PLAYA1 32 76 16 76 7
    twiddle PLAYA1 32 128
    makelump PLAYB1 30 77 15 77 9
    twiddle PLAYB1 32 128
    makelump PLAYC1 34 75 17 75 11
    twiddle PLAYC1 64 128
    makelump PLAYD1 37 78 18 78 13
    twiddle PLAYD1 64 128
    makelump PLAYE1 31 81 15 81 15
    twiddle PLAYE1 32 128
    makelump PLAYF1 30 80 15 80 17
    twiddle PLAYF1 32 128
    makelump PLAYG1 32 74 16 74 19
    twiddle PLAYG1 32 128
    makelump PLAYA2A8 48 84 24 84 21
    twiddle PLAYA2A8 64 128
    makelump PLAYB2B8 40 87 20 87 23
    twiddle PLAYB2B8 64 128
    makelump PLAYC2C8 46 84 23 84 25
    twiddle PLAYC2C8 64 128
    makelump PLAYD2D8 64 87 32 87 27
    twiddle PLAYD2D8 64 128
    makelump PLAYE2E8 47 83 23 83 29
    twiddle PLAYE2E8 64 128
    makelump PLAYF2F8 51 82 25 82 31
    twiddle PLAYF2F8 64 128
    makelump PLAYG2G8 48 73 24 73 33
    twiddle PLAYG2G8 64 128
    makelump PLAYA3A7 52 80 26 80 35
    twiddle PLAYA3A7 64 128
    makelump PLAYB3B7 69 83 34 83 37
    twiddle PLAYB3B7 128 128
    makelump PLAYC3C7 52 81 26 81 39
    twiddle PLAYC3C7 64 128
    makelump PLAYD3D7 75 84 37 84 41
    twiddle PLAYD3D7 128 128
    makelump PLAYE3E7 64 85 32 85 43
    twiddle PLAYE3E7 64 128
    makelump PLAYF3F7 65 85 32 85 45
    twiddle PLAYF3F7 128 128
    makelump PLAYG3G7 47 79 23 79 47
    twiddle PLAYG3G7 64 128
    makelump PLAYA4A6 35 75 17 75 49
    twiddle PLAYA4A6 64 128
    makelump PLAYB4B6 50 78 25 78 51
    twiddle PLAYB4B6 64 128
    makelump PLAYC4C6 41 76 20 76 53
    twiddle PLAYC4C6 64 128
    makelump PLAYD4D6 49 80 24 80 55
    twiddle PLAYD4D6 64 128
    makelump PLAYE4E6 42 83 21 83 57
    twiddle PLAYE4E6 64 128
    makelump PLAYF4F6 43 83 21 83 59
    twiddle PLAYF4F6 64 128
    makelump PLAYG4G6 31 81 15 81 61
    twiddle PLAYG4G6 32 128
    makelump PLAYA5 31 79 15 79 63
    twiddle PLAYA5 32 128
    makelump PLAYB5 30 83 15 83 65
    twiddle PLAYB5 32 128
    makelump PLAYC5 33 80 16 80 67
    twiddle PLAYC5 64 128
    makelump PLAYD5 35 84 17 84 69
    twiddle PLAYD5 64 128
    makelump PLAYE5 31 83 15 83 71
    twiddle PLAYE5 32 128
    makelump PLAYF5 30 83 15 83 73
    twiddle PLAYF5 32 128
    makelump PLAYG5 37 83 18 83 75
    twiddle PLAYG5 64 128
    makelump PLAYH0 40 63 20 63 77
    twiddle PLAYH0 64 64
    makelump PLAYI0 42 66 21 66 79
    twiddle PLAYI0 64 128
    makelump PLAYJ0 43 65 21 65 81
    twiddle PLAYJ0 64 128
    makelump PLAYK0 43 57 21 57 83
    twiddle PLAYK0 64 64
    makelump PLAYL0 48 43 24 43 85
    twiddle PLAYL0 64 64
    makelump PLAYM0 53 108 26 108 87
    twiddle PLAYM0 64 128
    makelump PLAYN0 42 62 21 62 89
    twiddle PLAYN0 64 64
    makelump PLAYO0 44 63 22 63 91
    twiddle PLAYO0 64 64
    makelump PLAYP0 49 61 24 61 93
    twiddle PLAYP0 64 64
    makelump PLAYQ0 51 57 25 57 95
    twiddle PLAYQ0 64 64
    makelump PLAYR0 51 49 25 49 97
    twiddle PLAYR0 64 64
    makelump PLAYS0 51 44 25 44 99
    twiddle PLAYS0 64 64
    makelump PLAYT0 53 31 26 31 101
    twiddle PLAYT0 64 32
    makelump PLAYU0 53 24 26 24 103
    twiddle PLAYU0 64 32
    makelump PLAYV0 53 21 26 21 105
    twiddle PLAYV0 64 32
    makelump TROOA1 48 91 24 90 5
    twiddle TROOA1 64 128
    makelump TROOB1 45 86 26 85 7
    twiddle TROOB1 64 128
    makelump TROOC1 45 90 23 89 9
    twiddle TROOC1 64 128
    makelump TROOD1 45 87 21 86 11
    twiddle TROOD1 64 128
    makelump TROOE1 57 84 35 85 13
    twiddle TROOE1 64 128
    makelump TROOF1 67 91 42 91 15
    twiddle TROOF1 128 128
    makelump TROOG1 39 87 16 86 17
    twiddle TROOG1 64 128
    makelump TROOH1 57 92 36 92 19
    twiddle TROOH1 64 128
    makelump TROOI1 50 85 25 86 21
    twiddle TROOI1 64 128
    makelump TROOJ1 55 87 26 87 23
    twiddle TROOJ1 64 128
    makelump TROOK1 40 82 17 81 25
    twiddle TROOK1 64 128
    makelump TROOA2A8 44 91 25 91 27
    twiddle TROOA2A8 64 128
    makelump TROOB2B8 55 88 29 88 29
    twiddle TROOB2B8 64 128
    makelump TROOC2C8 39 91 23 91 31
    twiddle TROOC2C8 64 128
    makelump TROOD2D8 53 86 26 87 33
    twiddle TROOD2D8 64 128
    makelump TROOE2E8 70 82 27 82 35
    twiddle TROOE2E8 128 128
    makelump TROOF2F8 56 87 28 87 37
    twiddle TROOF2F8 64 128
    makelump TROOG2G8 72 82 44 82 39
    twiddle TROOG2G8 128 128
    makelump TROOH2H8 53 90 28 90 41
    twiddle TROOH2H8 64 128
    makelump TROOI2I8 61 83 28 83 43
    twiddle TROOI2I8 64 128
    makelump TROOJ2J8 60 82 32 82 45
    twiddle TROOJ2J8 64 128
    makelump TROOK2K8 67 77 39 77 47
    twiddle TROOK2K8 128 128
    makelump TROOA3A7 43 87 22 87 49
    twiddle TROOA3A7 64 128
    makelump TROOB3B7 64 85 30 84 51
    twiddle TROOB3B7 64 128
    makelump TROOC3C7 43 90 21 90 53
    twiddle TROOC3C7 64 128
    makelump TROOD3D7 68 81 30 82 55
    twiddle TROOD3D7 128 128
    makelump TROOE3E7 71 84 21 84 57
    twiddle TROOE3E7 128 128
    makelump TROOF3F7 49 87 21 87 59
    twiddle TROOF3F7 64 128
    makelump TROOG3G7 74 81 47 81 61
    twiddle TROOG3G7 128 128
    makelump TROOH3H7 51 92 28 92 63
    twiddle TROOH3H7 64 128
    makelump TROOI3I7 60 87 29 87 65
    twiddle TROOI3I7 64 128
    makelump TROOJ3J7 56 85 27 85 67
    twiddle TROOJ3J7 64 128
    makelump TROOK3K7 66 79 37 79 69
    twiddle TROOK3K7 128 128
    makelump TROOA4A6 39 85 22 85 71
    twiddle TROOA4A6 64 128
    makelump TROOB4B6 52 82 22 82 73
    twiddle TROOB4B6 64 128
    makelump TROOC4C6 44 88 26 87 75
    twiddle TROOC4C6 64 128
    makelump TROOD4D6 49 86 27 85 77
    twiddle TROOD4D6 64 128
    makelump TROOE4E6 42 86 17 85 79
    twiddle TROOE4E6 64 128
    makelump TROOF4F6 60 87 40 87 81
    twiddle TROOF4F6 64 128
    makelump TROOG4G6 45 82 24 82 83
    twiddle TROOG4G6 64 128
    makelump TROOH4H6 50 94 30 93 85
    twiddle TROOH4H6 64 128
    makelump TROOI4I6 39 87 13 87 87
    twiddle TROOI4I6 64 128
    makelump TROOJ4J6 47 88 28 88 89
    twiddle TROOJ4J6 64 128
    makelump TROOK4K6 51 79 34 79 91
    twiddle TROOK4K6 64 128
    makelump TROOA5 44 85 22 85 93
    twiddle TROOA5 64 128
    makelump TROOB5 39 83 20 83 95
    twiddle TROOB5 64 128
    makelump TROOC5 44 86 23 86 97
    twiddle TROOC5 64 128
    makelump TROOD5 40 83 20 83 99
    twiddle TROOD5 64 128
    makelump TROOE5 65 87 19 87 101
    twiddle TROOE5 128 128
    makelump TROOF5 63 86 25 87 103
    twiddle TROOF5 64 128
    makelump TROOG5 47 79 28 80 105
    twiddle TROOG5 64 128
    makelump TROOH5 54 92 21 92 107
    twiddle TROOH5 64 128
    makelump TROOI5 52 87 26 87 109
    twiddle TROOI5 64 128
    makelump TROOJ5 50 82 22 82 111
    twiddle TROOJ5 64 128
    makelump TROOK5 39 76 16 76 113
    twiddle TROOK5 64 128
    makelump TROOL0 59 92 32 92 115
    twiddle TROOL0 64 128
    makelump TROOM0 55 70 29 70 117
    twiddle TROOM0 64 128
    makelump TROON0 57 70 28 70 119
    twiddle TROON0 64 128
    makelump TROOO0 56 56 29 56 121
    twiddle TROOO0 64 64
    makelump TROOP0 52 36 27 33 123
    twiddle TROOP0 64 64
    makelump TROOQ0 63 92 34 92 125
    twiddle TROOQ0 64 128
    makelump TROOR0 70 91 37 91 127
    twiddle TROOR0 128 128
    makelump TROOS0 84 91 44 91 129
    twiddle TROOS0 128 128
    makelump TROOT0 84 74 42 79 131
    twiddle TROOT0 128 128
    makelump TROOU0 78 69 36 72 133
    twiddle TROOU0 128 128
    makelump TROOV0 76 48 34 48 135
    twiddle TROOV0 128 64
    makelump TROOW0 66 34 31 33 137
    twiddle TROOW0 128 64
    makelump TROOX0 66 21 31 20 139
    twiddle TROOX0 128 32
    makelump BOSSA1 61 101 29 101 5
    twiddle BOSSA1 64 128
    makelump BOSSA5 57 93 29 93 7
    twiddle BOSSA5 64 128
    makelump BOSSA4A6 50 97 32 97 9
    twiddle BOSSA4A6 64 128
    makelump BOSSA3A7 40 103 21 103 11
    twiddle BOSSA3A7 64 128
    makelump BOSSA2A8 53 103 27 103 13
    twiddle BOSSA2A8 64 128
    makelump BOSSB1 56 94 29 94 15
    twiddle BOSSB1 64 128
    makelump BOSSB5 59 90 31 90 17
    twiddle BOSSB5 64 128
    makelump BOSSB4B6 55 93 28 93 19
    twiddle BOSSB4B6 64 128
    makelump BOSSB3B7 63 97 34 97 21
    twiddle BOSSB3B7 64 128
    makelump BOSSB2B8 61 100 33 100 23
    twiddle BOSSB2B8 64 128
    makelump BOSSC1 56 99 28 99 25
    twiddle BOSSC1 64 128
    makelump BOSSC5 56 94 28 94 27
    twiddle BOSSC5 64 128
    makelump BOSSC4C6 47 97 29 97 29
    twiddle BOSSC4C6 64 128
    makelump BOSSC3C7 42 100 22 100 31
    twiddle BOSSC3C7 64 128
    makelump BOSSC2C8 56 103 32 103 33
    twiddle BOSSC2C8 64 128
    makelump BOSSD1 53 97 26 97 35
    twiddle BOSSD1 64 128
    makelump BOSSD5 56 92 26 92 37
    twiddle BOSSD5 64 128
    makelump BOSSD4D6 53 92 30 92 39
    twiddle BOSSD4D6 64 128
    makelump BOSSD3D7 67 96 33 96 41
    twiddle BOSSD3D7 128 128
    makelump BOSSD2D8 63 99 32 99 43
    twiddle BOSSD2D8 64 128
    makelump BOSSE1 67 95 28 95 45
    twiddle BOSSE1 128 128
    makelump BOSSE5 64 110 41 110 47
    twiddle BOSSE5 64 128
    makelump BOSSE4E6 47 109 26 109 49
    twiddle BOSSE4E6 64 128
    makelump BOSSE3E7 73 103 34 103 51
    twiddle BOSSE3E7 128 128
    makelump BOSSE2E8 75 99 35 99 53
    twiddle BOSSE2E8 128 128
    makelump BOSSF1 71 97 31 97 55
    twiddle BOSSF1 128 128
    makelump BOSSF5 67 90 38 90 57
    twiddle BOSSF5 128 128
    makelump BOSSF4F6 57 95 44 95 59
    twiddle BOSSF4F6 64 128
    makelump BOSSF3F7 62 96 31 96 61
    twiddle BOSSF3F7 64 128
    makelump BOSSF2F8 65 94 32 94 63
    twiddle BOSSF2F8 128 128
    makelump BOSSG1 47 95 27 95 65
    twiddle BOSSG1 64 128
    makelump BOSSG5 55 83 23 83 67
    twiddle BOSSG5 64 128
    makelump BOSSG4G6 65 88 38 88 69
    twiddle BOSSG4G6 128 128
    makelump BOSSG3G7 71 92 40 92 71
    twiddle BOSSG3G7 128 128
    makelump BOSSG2G8 72 92 38 92 73
    twiddle BOSSG2G8 128 128
    makelump BOSSH1 60 94 26 94 75
    twiddle BOSSH1 64 128
    makelump BOSSH5 63 91 32 91 77
    twiddle BOSSH5 64 128
    makelump BOSSH4H6 66 97 42 97 79
    twiddle BOSSH4H6 128 128
    makelump BOSSH3H7 58 95 27 95 81
    twiddle BOSSH3H7 64 128
    makelump BOSSH2H8 60 95 28 95 83
    twiddle BOSSH2H8 64 128
    makelump BOSSI0 65 91 33 91 85
    twiddle BOSSI0 128 128
    makelump BOSSJ0 64 82 32 82 87
    twiddle BOSSJ0 64 128
    makelump BOSSK0 58 74 28 74 89
    twiddle BOSSK0 64 128
    makelump BOSSL0 54 61 26 61 91
    twiddle BOSSL0 64 64
    makelump BOSSM0 54 48 27 48 93
    twiddle BOSSM0 64 64
    makelump BOSSN0 58 34 29 33 95
    twiddle BOSSN0 64 64
    makelump FATTA1 109 104 67 103 3
    twiddle FATTA1 128 128
    makelump FATTB1 106 105 50 104 5
    twiddle FATTB1 128 128
    makelump FATTC1 115 104 47 103 7
    twiddle FATTC1 128 128
    makelump FATTD1 112 104 45 103 9
    twiddle FATTD1 128 128
    makelump FATTE1 112 103 55 102 11
    twiddle FATTE1 128 128
    makelump FATTF1 118 103 65 102 13
    twiddle FATTF1 128 128
    makelump FATTG1 109 106 54 105 15
    twiddle FATTG1 128 128
    makelump FATTH1 105 108 52 107 17
    twiddle FATTH1 128 128
    makelump FATTI1 108 108 59 107 19
    twiddle FATTI1 128 128
    makelump FATTA2A8 86 105 56 104 21
    twiddle FATTA2A8 128 128
    makelump FATTB2B8 106 109 69 108 23
    twiddle FATTB2B8 128 128
    makelump FATTC2C8 115 107 63 106 25
    twiddle FATTC2C8 128 128
    makelump FATTD2D8 110 106 60 105 27
    twiddle FATTD2D8 128 128
    makelump FATTE2E8 104 105 67 104 29
    twiddle FATTE2E8 128 128
    makelump FATTF2F8 89 105 57 104 31
    twiddle FATTF2F8 128 128
    makelump FATTG2G8 103 110 73 109 33
    twiddle FATTG2G8 128 128
    makelump FATTH2H8 101 111 57 110 35
    twiddle FATTH2H8 128 128
    makelump FATTI2I8 85 108 57 107 37
    twiddle FATTI2I8 128 128
    makelump FATTA3A7 95 107 56 106 39
    twiddle FATTA3A7 128 128
    makelump FATTB3B7 78 111 48 110 41
    twiddle FATTB3B7 128 128
    makelump FATTC3C7 102 111 61 110 43
    twiddle FATTC3C7 128 128
    makelump FATTD3D7 98 107 58 106 45
    twiddle FATTD3D7 128 128
    makelump FATTE3E7 89 106 57 105 47
    twiddle FATTE3E7 128 128
    makelump FATTF3F7 104 104 67 103 49
    twiddle FATTF3F7 128 128
    makelump FATTG3G7 102 109 64 108 51
    twiddle FATTG3G7 128 128
    makelump FATTH3H7 68 110 31 109 53
    twiddle FATTH3H7 128 128
    makelump FATTI3I7 98 108 56 107 55
    twiddle FATTI3I7 128 128
    makelump FATTA4A6 96 104 55 103 57
    twiddle FATTA4A6 128 128
    makelump FATTB4B6 94 106 62 105 59
    twiddle FATTB4B6 128 128
    makelump FATTC4C6 97 107 69 106 61
    twiddle FATTC4C6 128 128
    makelump FATTD4D6 96 105 70 104 63
    twiddle FATTD4D6 128 128
    makelump FATTE4E6 95 104 66 103 65
    twiddle FATTE4E6 128 128
    makelump FATTF4F6 99 102 61 101 67
    twiddle FATTF4F6 128 128
    makelump FATTG4G6 118 104 76 103 69
    twiddle FATTG4G6 128 128
    makelump FATTH4H6 97 107 54 106 71
    twiddle FATTH4H6 128 128
    makelump FATTI4I6 103 106 60 105 73
    twiddle FATTI4I6 128 128
    makelump FATTA5 96 105 36 104 75
    twiddle FATTA5 128 128
    makelump FATTB5 93 106 47 105 77
    twiddle FATTB5 128 128
    makelump FATTC5 106 104 65 103 79
    twiddle FATTC5 128 128
    makelump FATTD5 103 104 62 103 81
    twiddle FATTD5 128 128
    makelump FATTE5 96 103 51 102 83
    twiddle FATTE5 128 128
    makelump FATTF5 100 103 45 102 85
    twiddle FATTF5 128 128
    makelump FATTG5 94 104 47 103 87
    twiddle FATTG5 128 128
    makelump FATTH5 108 106 56 105 89
    twiddle FATTH5 128 128
    makelump FATTI5 92 105 35 104 91
    twiddle FATTI5 128 128
    makelump FATTJ0 108 105 50 105 93
    twiddle FATTJ0 128 128
    makelump FATTK0 111 96 52 96 95
    twiddle FATTK0 128 128
    makelump FATTL0 117 80 57 80 97
    twiddle FATTL0 128 128
    makelump FATTM0 117 79 55 77 99
    twiddle FATTM0 128 128
    makelump FATTN0 113 74 53 72 101
    twiddle FATTN0 128 128
    makelump FATTO0 113 70 52 69 103
    twiddle FATTO0 128 128
    makelump SKULA1 54 62 27 72 3
    twiddle SKULA1 64 64
    makelump SKULB1 54 63 27 73 5
    twiddle SKULB1 64 64
    makelump SKULC1 54 58 27 68 7
    twiddle SKULC1 64 64
    makelump SKULD1 53 64 26 71 9
    twiddle SKULD1 64 64
    makelump SKULE1 53 66 26 73 11
    twiddle SKULE1 64 128
    makelump SKULF1 49 59 25 67 13
    twiddle SKULF1 64 64
    makelump SKULA2A8 48 61 25 71 15
    twiddle SKULA2A8 64 64
    makelump SKULB2B8 46 62 25 72 17
    twiddle SKULB2B8 64 64
    makelump SKULC2C8 47 68 25 78 19
    twiddle SKULC2C8 64 128
    makelump SKULD2D8 48 64 24 73 21
    twiddle SKULD2D8 64 64
    makelump SKULE2E8 51 69 24 78 23
    twiddle SKULE2E8 64 128
    makelump SKULF2F8 43 63 16 73 25
    twiddle SKULF2F8 64 64
    makelump SKULA3A7 48 72 24 83 27
    twiddle SKULA3A7 64 128
    makelump SKULB3B7 48 71 24 82 29
    twiddle SKULB3B7 64 128
    makelump SKULC3C7 48 71 24 82 31
    twiddle SKULC3C7 64 128
    makelump SKULD3D7 44 76 19 86 33
    twiddle SKULD3D7 64 128
    makelump SKULE3E7 44 71 19 81 35
    twiddle SKULE3E7 64 128
    makelump SKULF3F7 47 67 20 78 37
    twiddle SKULF3F7 64 128
    makelump SKULA4A6 49 59 29 69 39
    twiddle SKULA4A6 64 64
    makelump SKULB4B6 50 63 29 73 41
    twiddle SKULB4B6 64 64
    makelump SKULC4C6 51 64 29 74 43
    twiddle SKULC4C6 64 64
    makelump SKULD4D6 61 72 27 82 45
    twiddle SKULD4D6 64 128
    makelump SKULE4E6 49 79 27 89 47
    twiddle SKULE4E6 64 128
    makelump SKULF4F6 47 61 23 72 49
    twiddle SKULF4F6 64 64
    makelump SKULA5 49 59 24 69 51
    twiddle SKULA5 64 64
    makelump SKULB5 49 62 24 72 53
    twiddle SKULB5 64 64
    makelump SKULC5 49 63 24 73 55
    twiddle SKULC5 64 64
    makelump SKULD5 53 68 26 75 57
    twiddle SKULD5 64 128
    makelump SKULE5 53 70 26 77 59
    twiddle SKULE5 64 128
    makelump SKULF5 47 58 19 68 61
    twiddle SKULF5 64 64
    makelump SKULG0 49 59 26 67 63
    twiddle SKULG0 64 64
    makelump SKULH0 70 74 29 76 65
    twiddle SKULH0 128 128
    makelump SKULI0 65 72 28 78 67
    twiddle SKULI0 128 128
    makelump SKULJ0 68 68 29 73 69
    twiddle SKULJ0 128 128
    makelump SKULK0 68 66 30 73 71
    twiddle SKULK0 128 128
    makelump SKULL0 64 65 29 70 73
    twiddle SKULL0 64 128
    makelump SKULM0 70 57 31 66 75
    twiddle SKULM0 128 64
    makelump SKULN0 65 60 30 71 77
    twiddle SKULN0 128 64
    makelump SKULO0 64 49 29 61 79
    twiddle SKULO0 64 64
    makelump SKULP0 49 47 16 59 81
    twiddle SKULP0 64 64
    makelump PAINA1 108 102 54 122 3
    twiddle PAINA1 128 128
    makelump PAINB1 108 92 54 121 5
    twiddle PAINB1 128 128
    makelump PAINC1 109 90 54 122 7
    twiddle PAINC1 128 128
    makelump PAIND1 108 99 54 120 9
    twiddle PAIND1 128 128
    makelump PAINA2A8 93 97 46 120 11
    twiddle PAINA2A8 128 128
    makelump PAINB2B8 94 97 47 118 13
    twiddle PAINB2B8 128 128
    makelump PAINC2C8 95 91 48 118 15
    twiddle PAINC2C8 128 128
    makelump PAIND2D8 90 103 46 117 17
    twiddle PAIND2D8 128 128
    makelump PAINA3A7 99 102 43 117 19
    twiddle PAINA3A7 128 128
    makelump PAINB3B7 98 96 42 116 21
    twiddle PAINB3B7 128 128
    makelump PAINC3C7 98 90 42 116 23
    twiddle PAINC3C7 128 128
    makelump PAIND3D7 98 105 41 119 25
    twiddle PAIND3D7 128 128
    makelump PAINA4A6 104 102 52 113 27
    twiddle PAINA4A6 128 128
    makelump PAINB4B6 106 98 53 113 29
    twiddle PAINB4B6 128 128
    makelump PAINC4C6 109 93 53 114 31
    twiddle PAINC4C6 128 128
    makelump PAIND4D6 102 104 51 116 33
    twiddle PAIND4D6 128 128
    makelump PAINA5 97 107 48 116 35
    twiddle PAINA5 128 128
    makelump PAINB5 102 106 51 116 37
    twiddle PAINB5 128 128
    makelump PAINC5 105 106 53 116 39
    twiddle PAINC5 128 128
    makelump PAIND5 99 100 50 116 41
    twiddle PAIND5 128 128
    makelump PAINE0 111 96 55 122 43
    twiddle PAINE0 128 128
    makelump PAINF0 113 98 56 122 45
    twiddle PAINF0 128 128
    makelump PAING0 115 95 57 122 47
    twiddle PAING0 128 128
    makelump PAINH0 110 95 56 127 49
    twiddle PAINH0 128 128
    makelump PAINI0 110 89 56 121 51
    twiddle PAINI0 128 128
    makelump PAINJ0 111 89 56 119 53
    twiddle PAINJ0 128 128
    makelump PAINK0 64 79 31 113 55
    twiddle PAINK0 64 128
    makelump PAINL0 58 77 32 114 57
    twiddle PAINL0 64 128
    makelump BSPIA1 141 84 66 84 3
    twiddle BSPIA1 256 128
    makelump BSPIB1 129 79 64 79 5
    twiddle BSPIB1 256 128
    makelump BSPIC1 143 83 73 83 7
    twiddle BSPIC1 256 128
    makelump BSPID1 129 80 63 80 9
    twiddle BSPID1 256 128
    makelump BSPIE1 122 78 61 78 11
    twiddle BSPIE1 128 128
    makelump BSPIF1 122 85 61 85 13
    twiddle BSPIF1 128 128
    makelump BSPIA2A8 125 90 56 90 15
    twiddle BSPIA2A8 128 128
    makelump BSPIB2B8 128 86 59 86 17
    twiddle BSPIB2B8 128 128
    makelump BSPIC2C8 130 85 55 86 19
    twiddle BSPIC2C8 256 128
    makelump BSPID2D8 117 85 47 85 21
    twiddle BSPID2D8 128 128
    makelump BSPIE2E8 120 85 54 85 23
    twiddle BSPIE2E8 128 128
    makelump BSPIF2F8 119 94 58 94 25
    twiddle BSPIF2F8 128 128
    makelump BSPIA3A7 88 92 44 92 27
    twiddle BSPIA3A7 128 128
    makelump BSPIB3B7 96 89 43 89 29
    twiddle BSPIB3B7 128 128
    makelump BSPIC3C7 88 89 44 89 31
    twiddle BSPIC3C7 128 128
    makelump BSPID3D7 87 87 49 87 33
    twiddle BSPID3D7 128 128
    makelump BSPIE3E7 98 84 56 84 35
    twiddle BSPIE3E7 128 128
    makelump BSPIF3F7 81 99 38 99 37
    twiddle BSPIF3F7 128 128
    makelump BSPIA4A6 133 97 69 97 39
    twiddle BSPIA4A6 256 128
    makelump BSPIB4B6 117 95 60 95 41
    twiddle BSPIB4B6 128 128
    makelump BSPIC4C6 128 91 69 92 43
    twiddle BSPIC4C6 128 128
    makelump BSPID4D6 132 89 67 89 45
    twiddle BSPID4D6 256 128
    makelump BSPIE4E6 121 89 64 89 47
    twiddle BSPIE4E6 128 128
    makelump BSPIF4F6 120 103 60 103 49
    twiddle BSPIF4F6 128 128
    makelump BSPIA5 152 89 75 89 51
    twiddle BSPIA5 256 128
    makelump BSPIB5 135 88 65 88 53
    twiddle BSPIB5 256 128
    makelump BSPIC5 152 88 74 88 55
    twiddle BSPIC5 256 128
    makelump BSPID5 133 88 66 88 57
    twiddle BSPID5 256 128
    makelump BSPIE5 128 83 63 83 59
    twiddle BSPIE5 128 128
    makelump BSPIF5 128 95 63 95 61
    twiddle BSPIF5 128 128
    makelump BSPIG0 123 84 61 84 63
    twiddle BSPIG0 128 128
    makelump BSPIH0 121 75 60 75 65
    twiddle BSPIH0 128 128
    makelump BSPII0 121 69 60 69 67
    twiddle BSPII0 128 128
    makelump BSPIJ0 121 59 60 59 69
    twiddle BSPIJ0 128 64
    makelump BSPIK0 121 52 60 52 71
    twiddle BSPIK0 128 64
    makelump BSPIL0 117 51 58 51 73
    twiddle BSPIL0 128 64
    makelump POSSA1 38 80 18 80 5
    twiddle POSSA1 64 128
    makelump POSSB1 37 82 18 82 7
    twiddle POSSB1 64 128
    makelump POSSC1 35 79 17 79 9
    twiddle POSSC1 64 128
    makelump POSSD1 37 81 16 81 11
    twiddle POSSD1 64 128
    makelump POSSE1 34 77 15 77 13
    twiddle POSSE1 64 128
    makelump POSSF1 31 77 14 77 15
    twiddle POSSF1 32 128
    makelump POSSG1 42 71 21 71 17
    twiddle POSSG1 64 128
    makelump POSSA2A8 56 82 34 82 19
    twiddle POSSA2A8 64 128
    makelump POSSB2B8 52 85 31 85 21
    twiddle POSSB2B8 64 128
    makelump POSSC2C8 51 82 29 82 23
    twiddle POSSC2C8 64 128
    makelump POSSD2D8 53 85 31 85 25
    twiddle POSSD2D8 64 128
    makelump POSSE2E8 51 78 28 78 27
    twiddle POSSE2E8 64 128
    makelump POSSF2F8 56 79 31 79 29
    twiddle POSSF2F8 64 128
    makelump POSSG2G8 55 74 30 74 31
    twiddle POSSG2G8 64 128
    makelump POSSA3A7 60 81 29 81 33
    twiddle POSSA3A7 64 128
    makelump POSSB3B7 51 82 30 82 35
    twiddle POSSB3B7 64 128
    makelump POSSC3C7 62 81 32 81 37
    twiddle POSSC3C7 64 128
    makelump POSSD3D7 51 83 31 83 39
    twiddle POSSD3D7 64 128
    makelump POSSE3E7 62 75 34 75 41
    twiddle POSSE3E7 64 128
    makelump POSSF3F7 63 76 37 76 43
    twiddle POSSF3F7 64 128
    makelump POSSG3G7 44 74 18 74 45
    twiddle POSSG3G7 64 128
    makelump POSSA4A6 41 80 20 80 47
    twiddle POSSA4A6 64 128
    makelump POSSB4B6 33 84 21 84 49
    twiddle POSSB4B6 64 128
    makelump POSSC4C6 48 81 23 81 51
    twiddle POSSC4C6 64 128
    makelump POSSD4D6 33 84 20 84 53
    twiddle POSSD4D6 64 128
    makelump POSSE4E6 37 74 21 74 55
    twiddle POSSE4E6 64 128
    makelump POSSF4F6 41 75 27 75 57
    twiddle POSSF4F6 64 128
    makelump POSSG4G6 36 79 17 79 59
    twiddle POSSG4G6 64 128
    makelump POSSA5 36 79 18 79 61
    twiddle POSSA5 64 128
    makelump POSSB5 36 81 18 81 63
    twiddle POSSB5 64 128
    makelump POSSC5 34 77 17 77 65
    twiddle POSSC5 64 128
    makelump POSSD5 36 81 19 81 67
    twiddle POSSD5 64 128
    makelump POSSE5 34 72 17 72 69
    twiddle POSSE5 64 128
    makelump POSSF5 32 74 18 74 71
    twiddle POSSF5 32 128
    makelump POSSG5 43 78 21 78 73
    twiddle POSSG5 64 128
    makelump POSSH0 49 76 21 76 75
    twiddle POSSH0 64 128
    makelump POSSI0 50 65 24 65 77
    twiddle POSSI0 64 128
    makelump POSSJ0 48 49 24 49 79
    twiddle POSSJ0 64 64
    makelump POSSK0 48 34 24 34 81
    twiddle POSSK0 64 64
    makelump POSSL0 44 27 22 27 83
    twiddle POSSL0 64 32
    makelump POSSM0 49 78 21 78 85
    twiddle POSSM0 64 128
    makelump POSSN0 51 82 25 82 87
    twiddle POSSN0 64 128
    makelump POSSO0 60 81 30 81 89
    twiddle POSSO0 64 128
    makelump POSSP0 60 64 30 64 91
    twiddle POSSP0 64 64
    makelump POSSQ0 60 53 30 53 93
    twiddle POSSQ0 64 64
    makelump POSSR0 64 46 32 46 95
    twiddle POSSR0 64 64
    makelump POSSS0 64 36 32 36 97
    twiddle POSSS0 64 64
    makelump POSST0 64 24 32 24 99
    twiddle POSST0 64 32
    makelump POSSU0 66 19 33 19 101
    twiddle POSSU0 128 32
    makelump HEADA1 106 95 52 94 3
    twiddle HEADA1 128 128
    makelump HEADB1 102 93 49 95 5
    twiddle HEADB1 128 128
    makelump HEADC1 105 98 51 95 7
    twiddle HEADC1 128 128
    makelump HEADD1 103 94 53 95 9
    twiddle HEADD1 128 128
    makelump HEADE1 109 90 56 99 11
    twiddle HEADE1 128 128
    makelump HEADF1 116 85 59 101 13
    twiddle HEADF1 128 128
    makelump HEADG1 118 76 60 103 15
    twiddle HEADG1 128 128
    makelump HEADA2A8 98 104 46 97 17
    twiddle HEADA2A8 128 128
    makelump HEADB2B8 85 100 38 99 19
    twiddle HEADB2B8 128 128
    makelump HEADC2C8 72 102 34 97 21
    twiddle HEADC2C8 128 128
    makelump HEADD2D8 83 101 39 96 23
    twiddle HEADD2D8 128 128
    makelump HEADE2E8 88 90 48 94 25
    twiddle HEADE2E8 128 128
    makelump HEADF2F8 92 85 55 102 27
    twiddle HEADF2F8 128 128
    makelump HEADG2G8 96 80 59 107 29
    twiddle HEADG2G8 128 128
    makelump HEADA3A7 58 108 28 98 31
    twiddle HEADA3A7 64 128
    makelump HEADB3B7 58 103 27 99 33
    twiddle HEADB3B7 64 128
    makelump HEADC3C7 71 97 39 97 35
    twiddle HEADC3C7 128 128
    makelump HEADD3D7 58 99 27 96 37
    twiddle HEADD3D7 64 128
    makelump HEADE3E7 72 85 41 96 39
    twiddle HEADE3E7 128 128
    makelump HEADF3F7 80 84 50 99 41
    twiddle HEADF3F7 128 128
    makelump HEADG3G7 72 74 42 101 43
    twiddle HEADG3G7 128 128
    makelump HEADA4A6 66 90 35 95 45
    twiddle HEADA4A6 128 128
    makelump HEADB4B6 82 93 46 94 47
    twiddle HEADB4B6 128 128
    makelump HEADC4C6 90 88 50 94 49
    twiddle HEADC4C6 128 128
    makelump HEADD4D6 83 91 46 93 51
    twiddle HEADD4D6 128 128
    makelump HEADE4E6 82 83 48 95 53
    twiddle HEADE4E6 128 128
    makelump HEADF4F6 88 77 56 97 55
    twiddle HEADF4F6 128 128
    makelump HEADG4G6 89 71 56 100 57
    twiddle HEADG4G6 128 128
    makelump HEADA5 96 92 52 92 59
    twiddle HEADA5 128 128
    makelump HEADB5 99 84 50 92 61
    twiddle HEADB5 128 128
    makelump HEADC5 94 91 42 90 63
    twiddle HEADC5 128 128
    makelump HEADD5 98 83 49 91 65
    twiddle HEADD5 128 128
    makelump HEADE5 90 81 44 90 67
    twiddle HEADE5 128 128
    makelump HEADF5 87 75 42 93 69
    twiddle HEADF5 128 128
    makelump HEADG5 94 65 47 97 71
    twiddle HEADG5 128 128
    makelump HEADH0 120 98 60 98 73
    twiddle HEADH0 128 128
    makelump HEADI0 107 84 53 100 75
    twiddle HEADI0 128 128
    makelump HEADJ0 105 74 51 93 77
    twiddle HEADJ0 128 128
    makelump HEADK0 98 67 46 65 79
    twiddle HEADK0 128 128
    makelump HEADL0 88 60 45 57 81
    twiddle HEADL0 128 64
    makelump HEADM0 85 55 46 52 83
    twiddle HEADM0 128 64
    makelump CYBRA1 114 163 55 163 3
    twiddle CYBRA1 128 256
    makelump CYBRB1 112 165 54 165 5
    twiddle CYBRB1 128 256
    makelump CYBRC1 101 164 50 164 7
    twiddle CYBRC1 128 256
    makelump CYBRD1 113 166 58 166 9
    twiddle CYBRD1 128 256
    makelump CYBRE1 104 167 54 167 11
    twiddle CYBRE1 128 256
    makelump CYBRF1 109 166 56 166 13
    twiddle CYBRF1 128 256
    makelump CYBRA8 87 162 43 162 15
    twiddle CYBRA8 128 256
    makelump CYBRB8 92 164 46 164 17
    twiddle CYBRB8 128 256
    makelump CYBRC8 103 166 51 166 19
    twiddle CYBRC8 128 256
    makelump CYBRD8 92 167 47 167 21
    twiddle CYBRD8 128 256
    makelump CYBRE8 140 168 61 168 23
    twiddle CYBRE8 256 256
    makelump CYBRF8 121 168 59 168 25
    twiddle CYBRF8 128 256
    makelump CYBRA7 86 162 43 162 27
    twiddle CYBRA7 128 256
    makelump CYBRB7 61 163 30 163 29
    twiddle CYBRB7 64 256
    makelump CYBRC7 88 162 44 162 31
    twiddle CYBRC7 128 256
    makelump CYBRD7 63 168 31 168 33
    twiddle CYBRD7 64 256
    makelump CYBRE7 128 164 43 164 35
    twiddle CYBRE7 128 256
    makelump CYBRF7 101 164 43 164 37
    twiddle CYBRF7 128 256
    makelump CYBRA6 97 163 51 163 39
    twiddle CYBRA6 128 256
    makelump CYBRB6 86 164 43 164 41
    twiddle CYBRB6 128 256
    makelump CYBRC6 94 158 41 158 43
    twiddle CYBRC6 128 256
    makelump CYBRD6 89 165 44 165 45
    twiddle CYBRD6 128 256
    makelump CYBRE6 96 158 40 158 47
    twiddle CYBRE6 128 256
    makelump CYBRF6 96 157 40 157 49
    twiddle CYBRF6 128 256
    makelump CYBRA5 113 158 62 158 51
    twiddle CYBRA5 128 256
    makelump CYBRB5 110 162 55 162 53
    twiddle CYBRB5 128 256
    makelump CYBRC5 107 157 53 157 55
    twiddle CYBRC5 128 256
    makelump CYBRD5 108 160 54 160 57
    twiddle CYBRD5 128 256
    makelump CYBRE5 103 159 54 159 59
    twiddle CYBRE5 128 256
    makelump CYBRF5 112 159 60 159 61
    twiddle CYBRF5 128 256
    makelump CYBRA4 86 162 47 162 63
    twiddle CYBRA4 128 256
    makelump CYBRB4 97 168 53 168 65
    twiddle CYBRB4 128 256
    makelump CYBRC4 113 163 60 163 67
    twiddle CYBRC4 128 256
    makelump CYBRD4 96 159 52 159 69
    twiddle CYBRD4 128 256
    makelump CYBRE4 125 170 76 170 71
    twiddle CYBRE4 128 256
    makelump CYBRF4 110 170 66 170 73
    twiddle CYBRF4 128 256
    makelump CYBRA3 90 163 46 163 75
    twiddle CYBRA3 128 256
    makelump CYBRB3 69 167 34 167 77
    twiddle CYBRB3 128 256
    makelump CYBRC3 93 164 46 164 79
    twiddle CYBRC3 128 256
    makelump CYBRD3 72 159 36 159 81
    twiddle CYBRD3 128 256
    makelump CYBRE3 145 170 97 170 83
    twiddle CYBRE3 256 256
    makelump CYBRF3 116 170 70 170 85
    twiddle CYBRF3 128 256
    makelump CYBRA2 112 167 42 167 87
    twiddle CYBRA2 128 256
    makelump CYBRB2 103 170 42 170 89
    twiddle CYBRB2 128 256
    makelump CYBRC2 101 166 46 166 91
    twiddle CYBRC2 128 256
    makelump CYBRD2 103 162 46 162 93
    twiddle CYBRD2 128 256
    makelump CYBRE2 93 163 45 163 95
    twiddle CYBRE2 128 256
    makelump CYBRF2 94 163 45 163 97
    twiddle CYBRF2 128 256
    makelump CYBRG0 88 167 44 167 99
    twiddle CYBRG0 128 256
    makelump CYBRH0 93 162 49 162 101
    twiddle CYBRH0 128 256
    makelump CYBRI0 96 159 51 159 103
    twiddle CYBRI0 128 256
    makelump CYBRJ0 107 156 53 156 105
    twiddle CYBRJ0 128 256
    makelump CYBRK0 106 152 53 152 107
    twiddle CYBRK0 128 256
    makelump CYBRL0 105 153 56 153 109
    twiddle CYBRL0 128 256
    makelump CYBRM0 105 152 56 152 111
    twiddle CYBRM0 128 256
    makelump CYBRN0 98 142 49 142 113
    twiddle CYBRN0 128 256
    makelump CYBRO0 102 30 52 30 115
    twiddle CYBRO0 128 32
    makelump RECTA1 148 144 70 164 3
    twiddle RECTA1 256 256
    makelump RECTB1 142 143 63 163 5
    twiddle RECTB1 256 256
    makelump RECTC1 144 142 73 161 7
    twiddle RECTC1 256 256
    makelump RECTD1 143 142 76 162 9
    twiddle RECTD1 256 256
    makelump RECTE1 141 141 66 162 11
    twiddle RECTE1 256 256
    makelump RECTF1 155 149 79 167 13
    twiddle RECTF1 256 256
    makelump RECTG1 93 150 44 167 15
    twiddle RECTG1 128 256
    makelump RECTH1 142 156 69 176 17
    twiddle RECTH1 256 256
    makelump RECTA2A8 111 145 66 161 19
    twiddle RECTA2A8 128 256
    makelump RECTB2B8 112 145 65 161 21
    twiddle RECTB2B8 128 256
    makelump RECTC2C8 127 144 77 161 23
    twiddle RECTC2C8 128 256
    makelump RECTD2D8 120 144 74 161 25
    twiddle RECTD2D8 128 256
    makelump RECTE2E8 155 143 82 156 27
    twiddle RECTE2E8 256 256
    makelump RECTF2F8 144 163 63 173 29
    twiddle RECTF2F8 256 256
    makelump RECTG2G8 109 150 68 158 31
    twiddle RECTG2G8 128 256
    makelump RECTH2H8 110 152 62 161 33
    twiddle RECTH2H8 128 256
    makelump RECTA3A7 130 152 71 162 35
    twiddle RECTA3A7 256 256
    makelump RECTB3B7 131 151 72 162 37
    twiddle RECTB3B7 256 256
    makelump RECTC3C7 134 151 74 162 39
    twiddle RECTC3C7 256 256
    makelump RECTD3D7 131 151 72 163 41
    twiddle RECTD3D7 256 256
    makelump RECTE3E7 133 150 74 163 43
    twiddle RECTE3E7 256 256
    makelump RECTF3F7 134 161 74 173 45
    twiddle RECTF3F7 256 256
    makelump RECTG3G7 126 152 75 162 47
    twiddle RECTG3G7 128 256
    makelump RECTH3H7 130 152 74 160 49
    twiddle RECTH3H7 256 256
    makelump RECTA4A6 133 160 79 166 51
    twiddle RECTA4A6 256 256
    makelump RECTB4B6 132 160 80 165 53
    twiddle RECTB4B6 256 256
    makelump RECTC4C6 128 160 75 164 55
    twiddle RECTC4C6 128 256
    makelump RECTD4D6 120 159 65 165 57
    twiddle RECTD4D6 128 256
    makelump RECTE4E6 135 160 77 164 59
    twiddle RECTE4E6 256 256
    makelump RECTF4F6 121 164 61 165 61
    twiddle RECTF4F6 128 256
    makelump RECTG4G6 117 158 69 165 63
    twiddle RECTG4G6 128 256
    makelump RECTH4H6 124 160 67 167 65
    twiddle RECTH4H6 128 256
    makelump RECTA5 122 160 61 167 67
    twiddle RECTA5 128 256
    makelump RECTB5 119 160 70 166 69
    twiddle RECTB5 128 256
    makelump RECTC5 121 161 62 166 71
    twiddle RECTC5 128 256
    makelump RECTD5 116 160 58 166 73
    twiddle RECTD5 128 256
    makelump RECTE5 137 158 69 162 75
    twiddle RECTE5 256 256
    makelump RECTF5 157 162 78 163 77
    twiddle RECTF5 256 256
    makelump RECTG5 84 161 45 167 79
    twiddle RECTG5 128 256
    makelump RECTH5 133 161 68 172 81
    twiddle RECTH5 256 256
    makelump RECTI0 142 156 71 156 83
    twiddle RECTI0 256 256
    makelump RECTJ0 148 152 74 152 85
    twiddle RECTJ0 256 256
    makelump RECTK0 162 153 81 153 87
    twiddle RECTK0 256 256
    makelump RECTL0 172 150 86 150 89
    twiddle RECTL0 256 256
    makelump RECTM0 172 156 86 156 91
    twiddle RECTM0 256 256
    makelump RECTN0 176 153 88 153 93
    twiddle RECTN0 256 256
    makelump RECTO0 170 148 85 148 95
    twiddle RECTO0 256 256
    makelump ZOMBA1 38 80 18 80 5
    twiddle ZOMBA1 64 128
    makelump ZOMBB1 37 82 18 82 7
    twiddle ZOMBB1 64 128
    makelump ZOMBC1 35 79 17 79 9
    twiddle ZOMBC1 64 128
    makelump ZOMBD1 37 81 16 81 11
    twiddle ZOMBD1 64 128
    makelump ZOMBE1 34 77 15 77 13
    twiddle ZOMBE1 64 128
    makelump ZOMBF1 31 77 14 77 15
    twiddle ZOMBF1 32 128
    makelump ZOMBG1 42 71 21 71 17
    twiddle ZOMBG1 64 128
    makelump ZOMBA2A8 56 82 34 82 19
    twiddle ZOMBA2A8 64 128
    makelump ZOMBB2B8 52 85 31 85 21
    twiddle ZOMBB2B8 64 128
    makelump ZOMBC2C8 51 82 29 82 23
    twiddle ZOMBC2C8 64 128
    makelump ZOMBD2D8 53 85 31 85 25
    twiddle ZOMBD2D8 64 128
    makelump ZOMBE2E8 51 78 28 78 27
    twiddle ZOMBE2E8 64 128
    makelump ZOMBF2F8 56 79 31 79 29
    twiddle ZOMBF2F8 64 128
    makelump ZOMBG2G8 55 74 30 74 31
    twiddle ZOMBG2G8 64 128
    makelump ZOMBA3A7 60 81 29 81 33
    twiddle ZOMBA3A7 64 128
    makelump ZOMBB3B7 51 82 30 82 35
    twiddle ZOMBB3B7 64 128
    makelump ZOMBC3C7 62 81 32 81 37
    twiddle ZOMBC3C7 64 128
    makelump ZOMBD3D7 51 83 31 83 39
    twiddle ZOMBD3D7 64 128
    makelump ZOMBE3E7 62 75 34 75 41
    twiddle ZOMBE3E7 64 128
    makelump ZOMBF3F7 63 76 37 76 43
    twiddle ZOMBF3F7 64 128
    makelump ZOMBG3G7 44 74 18 74 45
    twiddle ZOMBG3G7 64 128
    makelump ZOMBA4A6 41 80 20 80 47
    twiddle ZOMBA4A6 64 128
    makelump ZOMBB4B6 33 84 21 84 49
    twiddle ZOMBB4B6 64 128
    makelump ZOMBC4C6 48 81 23 81 51
    twiddle ZOMBC4C6 64 128
    makelump ZOMBD4D6 33 84 20 84 53
    twiddle ZOMBD4D6 64 128
    makelump ZOMBE4E6 37 74 21 74 55
    twiddle ZOMBE4E6 64 128
    makelump ZOMBF4F6 41 75 27 75 57
    twiddle ZOMBF4F6 64 128
    makelump ZOMBG4G6 36 79 17 79 59
    twiddle ZOMBG4G6 64 128
    makelump ZOMBA5 36 79 18 79 61
    twiddle ZOMBA5 64 128
    makelump ZOMBB5 36 81 18 81 63
    twiddle ZOMBB5 64 128
    makelump ZOMBC5 34 77 17 77 65
    twiddle ZOMBC5 64 128
    makelump ZOMBD5 36 81 19 81 67
    twiddle ZOMBD5 64 128
    makelump ZOMBE5 34 72 17 72 69
    twiddle ZOMBE5 64 128
    makelump ZOMBF5 32 74 18 74 71
    twiddle ZOMBF5 32 128
    makelump ZOMBG5 43 78 21 78 73
    twiddle ZOMBG5 64 128
    makelump ZOMBH0 49 76 21 76 75
    twiddle ZOMBH0 64 128
    makelump ZOMBI0 50 65 24 65 77
    twiddle ZOMBI0 64 128
    makelump ZOMBJ0 48 49 24 49 79
    twiddle ZOMBJ0 64 64
    makelump ZOMBK0 48 34 24 34 81
    twiddle ZOMBK0 64 64
    makelump ZOMBL0 44 27 22 27 83
    twiddle ZOMBL0 64 32
    makelump ZOMBM0 49 78 21 78 85
    twiddle ZOMBM0 64 128
    makelump ZOMBN0 51 82 25 82 87
    twiddle ZOMBN0 64 128
    makelump ZOMBO0 60 81 30 81 89
    twiddle ZOMBO0 64 128
    makelump ZOMBP0 60 64 30 64 91
    twiddle ZOMBP0 64 64
    makelump ZOMBQ0 60 53 30 53 93
    twiddle ZOMBQ0 64 64
    makelump ZOMBR0 64 46 32 46 95
    twiddle ZOMBR0 64 64
    makelump ZOMBS0 64 36 32 36 97
    twiddle ZOMBS0 64 64
    makelump ZOMBT0 64 24 32 24 99
    twiddle ZOMBT0 64 32
    makelump ZOMBU0 66 19 33 19 101
    twiddle ZOMBU0 128 32
    makelump SPECA1 62 84 31 84 5
    twiddle SPECA1 64 128
    makelump SPECB1 59 79 29 79 7
    twiddle SPECB1 64 128
    makelump SPECC1 59 82 29 82 9
    twiddle SPECC1 64 128
    makelump SPECD1 62 80 31 80 11
    twiddle SPECD1 64 128
    makelump SPECE1 77 82 39 82 13
    twiddle SPECE1 128 128
    makelump SPECF1 76 84 38 84 15
    twiddle SPECF1 128 128
    makelump SPECG1 68 84 35 84 17
    twiddle SPECG1 128 128
    makelump SPECH1 61 90 30 90 19
    twiddle SPECH1 64 128
    makelump SPECA2A8 67 84 34 83 21
    twiddle SPECA2A8 128 128
    makelump SPECB2B8 62 77 35 78 23
    twiddle SPECB2B8 64 128
    makelump SPECC2C8 72 85 36 85 25
    twiddle SPECC2C8 128 128
    makelump SPECD2D8 62 80 35 80 27
    twiddle SPECD2D8 64 128
    makelump SPECE2E8 81 82 44 82 29
    twiddle SPECE2E8 128 128
    makelump SPECF2F8 87 84 58 84 31
    twiddle SPECF2F8 128 128
    makelump SPECG2G8 72 84 43 84 33
    twiddle SPECG2G8 128 128
    makelump SPECH2H8 52 93 27 92 35
    twiddle SPECH2H8 64 128
    makelump SPECA3A7 73 79 36 79 37
    twiddle SPECA3A7 128 128
    makelump SPECB3B7 63 77 33 77 39
    twiddle SPECB3B7 64 128
    makelump SPECC3C7 73 81 35 81 41
    twiddle SPECC3C7 128 128
    makelump SPECD3D7 63 77 33 78 43
    twiddle SPECD3D7 64 128
    makelump SPECE3E7 70 82 36 82 45
    twiddle SPECE3E7 128 128
    makelump SPECF3F7 75 83 40 83 47
    twiddle SPECF3F7 128 128
    makelump SPECG3G7 72 84 37 84 49
    twiddle SPECG3G7 128 128
    makelump SPECH3H7 58 90 26 90 51
    twiddle SPECH3H7 64 128
    makelump SPECA4A6 70 80 35 80 53
    twiddle SPECA4A6 128 128
    makelump SPECB4B6 63 77 31 77 55
    twiddle SPECB4B6 64 128
    makelump SPECC4C6 69 83 34 83 57
    twiddle SPECC4C6 128 128
    makelump SPECD4D6 57 75 29 75 59
    twiddle SPECD4D6 64 128
    makelump SPECE4E6 71 78 45 78 61
    twiddle SPECE4E6 128 128
    makelump SPECF4F6 74 81 49 80 63
    twiddle SPECF4F6 128 128
    makelump SPECG4G6 63 79 38 78 65
    twiddle SPECG4G6 64 128
    makelump SPECH4H6 64 84 31 84 67
    twiddle SPECH4H6 64 128
    makelump SPECA5 63 76 29 76 69
    twiddle SPECA5 64 128
    makelump SPECB5 58 79 29 79 71
    twiddle SPECB5 64 128
    makelump SPECC5 61 78 33 78 73
    twiddle SPECC5 64 128
    makelump SPECD5 63 77 32 77 75
    twiddle SPECD5 64 128
    makelump SPECE5 75 77 37 77 77
    twiddle SPECE5 128 128
    makelump SPECF5 74 79 38 79 79
    twiddle SPECF5 128 128
    makelump SPECG5 70 76 34 76 81
    twiddle SPECG5 128 128
    makelump SPECH5 62 83 30 83 83
    twiddle SPECH5 64 128
    makelump SPECI0 76 99 41 99 85
    twiddle SPECI0 128 128
    makelump SPECJ0 75 100 39 100 87
    twiddle SPECJ0 128 128
    makelump SPECK0 66 71 34 71 89
    twiddle SPECK0 128 128
    makelump SPECL0 67 75 36 75 91
    twiddle SPECL0 128 128
    makelump SPECM0 66 64 35 64 93
    twiddle SPECM0 128 64
    makelump SPECN0 62 42 31 41 95
    twiddle SPECN0 64 64
    makelump PLY1A1 32 76 16 76 7
    twiddle PLY1A1 32 128
    makelump PLY1B1 30 77 15 77 9
    twiddle PLY1B1 32 128
    makelump PLY1C1 34 75 17 75 11
    twiddle PLY1C1 64 128
    makelump PLY1D1 37 78 18 78 13
    twiddle PLY1D1 64 128
    makelump PLY1E1 31 81 15 81 15
    twiddle PLY1E1 32 128
    makelump PLY1F1 30 80 15 80 17
    twiddle PLY1F1 32 128
    makelump PLY1G1 32 74 16 74 19
    twiddle PLY1G1 32 128
    makelump PLY1A2A8 48 84 24 84 21
    twiddle PLY1A2A8 64 128
    makelump PLY1B2B8 40 87 20 87 23
    twiddle PLY1B2B8 64 128
    makelump PLY1C2C8 46 84 23 84 25
    twiddle PLY1C2C8 64 128
    makelump PLY1D2D8 64 87 32 87 27
    twiddle PLY1D2D8 64 128
    makelump PLY1E2E8 47 83 23 83 29
    twiddle PLY1E2E8 64 128
    makelump PLY1F2F8 51 82 25 82 31
    twiddle PLY1F2F8 64 128
    makelump PLY1G2G8 48 73 24 73 33
    twiddle PLY1G2G8 64 128
    makelump PLY1A3A7 52 80 26 80 35
    twiddle PLY1A3A7 64 128
    makelump PLY1B3B7 69 83 34 83 37
    twiddle PLY1B3B7 128 128
    makelump PLY1C3C7 52 81 26 81 39
    twiddle PLY1C3C7 64 128
    makelump PLY1D3D7 75 84 37 84 41
    twiddle PLY1D3D7 128 128
    makelump PLY1E3E7 64 85 32 85 43
    twiddle PLY1E3E7 64 128
    makelump PLY1F3F7 65 85 32 85 45
    twiddle PLY1F3F7 128 128
    makelump PLY1G3G7 47 79 23 79 47
    twiddle PLY1G3G7 64 128
    makelump PLY1A4A6 35 75 17 75 49
    twiddle PLY1A4A6 64 128
    makelump PLY1B4B6 50 78 25 78 51
    twiddle PLY1B4B6 64 128
    makelump PLY1C4C6 41 76 20 76 53
    twiddle PLY1C4C6 64 128
    makelump PLY1D4D6 49 80 24 80 55
    twiddle PLY1D4D6 64 128
    makelump PLY1E4E6 42 83 21 83 57
    twiddle PLY1E4E6 64 128
    makelump PLY1F4F6 43 83 21 83 59
    twiddle PLY1F4F6 64 128
    makelump PLY1G4G6 31 81 15 81 61
    twiddle PLY1G4G6 32 128
    makelump PLY1A5 31 79 15 79 63
    twiddle PLY1A5 32 128
    makelump PLY1B5 30 83 15 83 65
    twiddle PLY1B5 32 128
    makelump PLY1C5 33 80 16 80 67
    twiddle PLY1C5 64 128
    makelump PLY1D5 35 84 17 84 69
    twiddle PLY1D5 64 128
    makelump PLY1E5 31 83 15 83 71
    twiddle PLY1E5 32 128
    makelump PLY1F5 30 83 15 83 73
    twiddle PLY1F5 32 128
    makelump PLY1G5 37 83 18 83 75
    twiddle PLY1G5 64 128
    makelump PLY1H0 40 63 20 63 77
    twiddle PLY1H0 64 64
    makelump PLY1I0 42 66 21 66 79
    twiddle PLY1I0 64 128
    makelump PLY1J0 43 65 21 65 81
    twiddle PLY1J0 64 128
    makelump PLY1K0 43 57 21 57 83
    twiddle PLY1K0 64 64
    makelump PLY1L0 48 43 24 43 85
    twiddle PLY1L0 64 64
    makelump PLY1M0 53 108 26 108 87
    twiddle PLY1M0 64 128
    makelump PLY1N0 42 62 21 62 89
    twiddle PLY1N0 64 64
    makelump PLY1O0 44 63 22 63 91
    twiddle PLY1O0 64 64
    makelump PLY1P0 49 61 24 61 93
    twiddle PLY1P0 64 64
    makelump PLY1Q0 51 57 25 57 95
    twiddle PLY1Q0 64 64
    makelump PLY1R0 51 49 25 49 97
    twiddle PLY1R0 64 64
    makelump PLY1S0 51 44 25 44 99
    twiddle PLY1S0 64 64
    makelump PLY1T0 53 31 26 31 101
    twiddle PLY1T0 64 32
    makelump PLY1U0 53 24 26 24 103
    twiddle PLY1U0 64 32
    makelump PLY1V0 53 21 26 21 105
    twiddle PLY1V0 64 32
    makelump PLY2A1 32 76 16 76 7
    twiddle PLY2A1 32 128
    makelump PLY2B1 30 77 15 77 9
    twiddle PLY2B1 32 128
    makelump PLY2C1 34 75 17 75 11
    twiddle PLY2C1 64 128
    makelump PLY2D1 37 78 18 78 13
    twiddle PLY2D1 64 128
    makelump PLY2E1 31 81 15 81 15
    twiddle PLY2E1 32 128
    makelump PLY2F1 30 80 15 80 17
    twiddle PLY2F1 32 128
    makelump PLY2G1 32 74 16 74 19
    twiddle PLY2G1 32 128
    makelump PLY2A2A8 48 84 24 84 21
    twiddle PLY2A2A8 64 128
    makelump PLY2B2B8 40 87 20 87 23
    twiddle PLY2B2B8 64 128
    makelump PLY2C2C8 46 84 23 84 25
    twiddle PLY2C2C8 64 128
    makelump PLY2D2D8 64 87 32 87 27
    twiddle PLY2D2D8 64 128
    makelump PLY2E2E8 47 83 23 83 29
    twiddle PLY2E2E8 64 128
    makelump PLY2F2F8 51 82 25 82 31
    twiddle PLY2F2F8 64 128
    makelump PLY2G2G8 48 73 24 73 33
    twiddle PLY2G2G8 64 128
    makelump PLY2A3A7 52 80 26 80 35
    twiddle PLY2A3A7 64 128
    makelump PLY2B3B7 69 83 34 83 37
    twiddle PLY2B3B7 128 128
    makelump PLY2C3C7 52 81 26 81 39
    twiddle PLY2C3C7 64 128
    makelump PLY2D3D7 75 84 37 84 41
    twiddle PLY2D3D7 128 128
    makelump PLY2E3E7 64 85 32 85 43
    twiddle PLY2E3E7 64 128
    makelump PLY2F3F7 65 85 32 85 45
    twiddle PLY2F3F7 128 128
    makelump PLY2G3G7 47 79 23 79 47
    twiddle PLY2G3G7 64 128
    makelump PLY2A4A6 35 75 17 75 49
    twiddle PLY2A4A6 64 128
    makelump PLY2B4B6 50 78 25 78 51
    twiddle PLY2B4B6 64 128
    makelump PLY2C4C6 41 76 20 76 53
    twiddle PLY2C4C6 64 128
    makelump PLY2D4D6 49 80 24 80 55
    twiddle PLY2D4D6 64 128
    makelump PLY2E4E6 42 83 21 83 57
    twiddle PLY2E4E6 64 128
    makelump PLY2F4F6 43 83 21 83 59
    twiddle PLY2F4F6 64 128
    makelump PLY2G4G6 31 81 15 81 61
    twiddle PLY2G4G6 32 128
    makelump PLY2A5 31 79 15 79 63
    twiddle PLY2A5 32 128
    makelump PLY2B5 30 83 15 83 65
    twiddle PLY2B5 32 128
    makelump PLY2C5 33 80 16 80 67
    twiddle PLY2C5 64 128
    makelump PLY2D5 35 84 17 84 69
    twiddle PLY2D5 64 128
    makelump PLY2E5 31 83 15 83 71
    twiddle PLY2E5 32 128
    makelump PLY2F5 30 83 15 83 73
    twiddle PLY2F5 32 128
    makelump PLY2G5 37 83 18 83 75
    twiddle PLY2G5 64 128
    makelump PLY2H0 40 63 20 63 77
    twiddle PLY2H0 64 64
    makelump PLY2I0 42 66 21 66 79
    twiddle PLY2I0 64 128
    makelump PLY2J0 43 65 21 65 81
    twiddle PLY2J0 64 128
    makelump PLY2K0 43 57 21 57 83
    twiddle PLY2K0 64 64
    makelump PLY2L0 48 43 24 43 85
    twiddle PLY2L0 64 64
    makelump PLY2M0 53 108 26 108 87
    twiddle PLY2M0 64 128
    makelump PLY2N0 42 62 21 62 89
    twiddle PLY2N0 64 64
    makelump PLY2O0 44 63 22 63 91
    twiddle PLY2O0 64 64
    makelump PLY2P0 49 61 24 61 93
    twiddle PLY2P0 64 64
    makelump PLY2Q0 51 57 25 57 95
    twiddle PLY2Q0 64 64
    makelump PLY2R0 51 49 25 49 97
    twiddle PLY2R0 64 64
    makelump PLY2S0 51 44 25 44 99
    twiddle PLY2S0 64 64
    makelump PLY2T0 53 31 26 31 101
    twiddle PLY2T0 64 32
    makelump PLY2U0 53 24 26 24 103
    twiddle PLY2U0 64 32
    makelump PLY2V0 53 21 26 21 105
    twiddle PLY2V0 64 32
    makelump NITEA1 48 91 24 90 5
    twiddle NITEA1 64 128
    makelump NITEB1 45 86 26 85 7
    twiddle NITEB1 64 128
    makelump NITEC1 45 90 23 89 9
    twiddle NITEC1 64 128
    makelump NITED1 45 87 21 86 11
    twiddle NITED1 64 128
    makelump NITEE1 57 84 35 85 13
    twiddle NITEE1 64 128
    makelump NITEF1 67 91 42 91 15
    twiddle NITEF1 128 128
    makelump NITEG1 39 87 16 86 17
    twiddle NITEG1 64 128
    makelump NITEH1 57 92 36 92 19
    twiddle NITEH1 64 128
    makelump NITEI1 50 85 25 86 21
    twiddle NITEI1 64 128
    makelump NITEJ1 55 87 26 87 23
    twiddle NITEJ1 64 128
    makelump NITEK1 40 82 17 81 25
    twiddle NITEK1 64 128
    makelump NITEA2A8 44 91 25 91 27
    twiddle NITEA2A8 64 128
    makelump NITEB2B8 55 88 29 88 29
    twiddle NITEB2B8 64 128
    makelump NITEC2C8 39 91 23 91 31
    twiddle NITEC2C8 64 128
    makelump NITED2D8 53 86 26 87 33
    twiddle NITED2D8 64 128
    makelump NITEE2E8 70 82 27 82 35
    twiddle NITEE2E8 128 128
    makelump NITEF2F8 56 87 28 87 37
    twiddle NITEF2F8 64 128
    makelump NITEG2G8 72 82 44 82 39
    twiddle NITEG2G8 128 128
    makelump NITEH2H8 53 90 28 90 41
    twiddle NITEH2H8 64 128
    makelump NITEI2I8 61 83 28 83 43
    twiddle NITEI2I8 64 128
    makelump NITEJ2J8 60 82 32 82 45
    twiddle NITEJ2J8 64 128
    makelump NITEK2K8 67 77 39 77 47
    twiddle NITEK2K8 128 128
    makelump NITEA3A7 43 87 22 87 49
    twiddle NITEA3A7 64 128
    makelump NITEB3B7 64 85 30 84 51
    twiddle NITEB3B7 64 128
    makelump NITEC3C7 43 90 21 90 53
    twiddle NITEC3C7 64 128
    makelump NITED3D7 68 81 30 82 55
    twiddle NITED3D7 128 128
    makelump NITEE3E7 71 84 21 84 57
    twiddle NITEE3E7 128 128
    makelump NITEF3F7 49 87 21 87 59
    twiddle NITEF3F7 64 128
    makelump NITEG3G7 74 81 47 81 61
    twiddle NITEG3G7 128 128
    makelump NITEH3H7 51 92 28 92 63
    twiddle NITEH3H7 64 128
    makelump NITEI3I7 60 87 29 87 65
    twiddle NITEI3I7 64 128
    makelump NITEJ3J7 56 85 27 85 67
    twiddle NITEJ3J7 64 128
    makelump NITEK3K7 66 79 37 79 69
    twiddle NITEK3K7 128 128
    makelump NITEA4A6 39 85 22 85 71
    twiddle NITEA4A6 64 128
    makelump NITEB4B6 52 82 22 82 73
    twiddle NITEB4B6 64 128
    makelump NITEC4C6 44 88 26 87 75
    twiddle NITEC4C6 64 128
    makelump NITED4D6 49 86 27 85 77
    twiddle NITED4D6 64 128
    makelump NITEE4E6 42 86 17 85 79
    twiddle NITEE4E6 64 128
    makelump NITEF4F6 60 87 40 87 81
    twiddle NITEF4F6 64 128
    makelump NITEG4G6 45 82 24 82 83
    twiddle NITEG4G6 64 128
    makelump NITEH4H6 50 94 30 93 85
    twiddle NITEH4H6 64 128
    makelump NITEI4I6 39 87 13 87 87
    twiddle NITEI4I6 64 128
    makelump NITEJ4J6 47 88 28 88 89
    twiddle NITEJ4J6 64 128
    makelump NITEK4K6 51 79 34 79 91
    twiddle NITEK4K6 64 128
    makelump NITEA5 44 85 22 85 93
    twiddle NITEA5 64 128
    makelump NITEB5 39 83 20 83 95
    twiddle NITEB5 64 128
    makelump NITEC5 44 86 23 86 97
    twiddle NITEC5 64 128
    makelump NITED5 40 83 20 83 99
    twiddle NITED5 64 128
    makelump NITEE5 65 87 19 87 101
    twiddle NITEE5 128 128
    makelump NITEF5 63 86 25 87 103
    twiddle NITEF5 64 128
    makelump NITEG5 47 79 28 80 105
    twiddle NITEG5 64 128
    makelump NITEH5 54 92 21 92 107
    twiddle NITEH5 64 128
    makelump NITEI5 52 87 26 87 109
    twiddle NITEI5 64 128
    makelump NITEJ5 50 82 22 82 111
    twiddle NITEJ5 64 128
    makelump NITEK5 39 76 16 76 113
    twiddle NITEK5 64 128
    makelump NITEL0 59 92 32 92 115
    twiddle NITEL0 64 128
    makelump NITEM0 55 70 29 70 117
    twiddle NITEM0 64 128
    makelump NITEN0 57 70 28 70 119
    twiddle NITEN0 64 128
    makelump NITEO0 56 56 29 56 121
    twiddle NITEO0 64 64
    makelump NITEP0 52 36 27 33 123
    twiddle NITEP0 64 64
    makelump NITEQ0 63 92 34 92 125
    twiddle NITEQ0 64 128
    makelump NITER0 70 91 37 91 127
    twiddle NITER0 128 128
    makelump NITES0 84 91 44 91 129
    twiddle NITES0 128 128
    makelump NITET0 84 74 42 79 131
    twiddle NITET0 128 128
    makelump NITEU0 78 69 36 72 133
    twiddle NITEU0 128 128
    makelump NITEV0 76 48 34 48 135
    twiddle NITEV0 128 64
    makelump NITEW0 66 34 31 33 137
    twiddle NITEW0 128 64
    makelump NITEX0 66 21 31 20 139
    twiddle NITEX0 128 32
    makelump BAROA1 61 101 29 101 5
    twiddle BAROA1 64 128
    makelump BAROA5 57 93 29 93 7
    twiddle BAROA5 64 128
    makelump BAROA4A6 50 97 32 97 9
    twiddle BAROA4A6 64 128
    makelump BAROA3A7 40 103 21 103 11
    twiddle BAROA3A7 64 128
    makelump BAROA2A8 53 103 27 103 13
    twiddle BAROA2A8 64 128
    makelump BAROB1 56 94 29 94 15
    twiddle BAROB1 64 128
    makelump BAROB5 59 90 31 90 17
    twiddle BAROB5 64 128
    makelump BAROB4B6 55 93 28 93 19
    twiddle BAROB4B6 64 128
    makelump BAROB3B7 63 97 34 97 21
    twiddle BAROB3B7 64 128
    makelump BAROB2B8 61 100 33 100 23
    twiddle BAROB2B8 64 128
    makelump BAROC1 56 99 28 99 25
    twiddle BAROC1 64 128
    makelump BAROC5 56 94 28 94 27
    twiddle BAROC5 64 128
    makelump BAROC4C6 47 97 29 97 29
    twiddle BAROC4C6 64 128
    makelump BAROC3C7 42 100 22 100 31
    twiddle BAROC3C7 64 128
    makelump BAROC2C8 56 103 32 103 33
    twiddle BAROC2C8 64 128
    makelump BAROD1 53 97 26 97 35
    twiddle BAROD1 64 128
    makelump BAROD5 56 92 26 92 37
    twiddle BAROD5 64 128
    makelump BAROD4D6 53 92 30 92 39
    twiddle BAROD4D6 64 128
    makelump BAROD3D7 67 96 33 96 41
    twiddle BAROD3D7 128 128
    makelump BAROD2D8 63 99 32 99 43
    twiddle BAROD2D8 64 128
    makelump BAROE1 67 95 28 95 45
    twiddle BAROE1 128 128
    makelump BAROE5 64 110 41 110 47
    twiddle BAROE5 64 128
    makelump BAROE4E6 47 109 26 109 49
    twiddle BAROE4E6 64 128
    makelump BAROE3E7 73 103 34 103 51
    twiddle BAROE3E7 128 128
    makelump BAROE2E8 75 99 35 99 53
    twiddle BAROE2E8 128 128
    makelump BAROF1 71 97 31 97 55
    twiddle BAROF1 128 128
    makelump BAROF5 67 90 38 90 57
    twiddle BAROF5 128 128
    makelump BAROF4F6 57 95 44 95 59
    twiddle BAROF4F6 64 128
    makelump BAROF3F7 62 96 31 96 61
    twiddle BAROF3F7 64 128
    makelump BAROF2F8 65 94 32 94 63
    twiddle BAROF2F8 128 128
    makelump BAROG1 47 95 27 95 65
    twiddle BAROG1 64 128
    makelump BAROG5 55 83 23 83 67
    twiddle BAROG5 64 128
    makelump BAROG4G6 65 88 38 88 69
    twiddle BAROG4G6 128 128
    makelump BAROG3G7 71 92 40 92 71
    twiddle BAROG3G7 128 128
    makelump BAROG2G8 72 92 38 92 73
    twiddle BAROG2G8 128 128
    makelump BAROH1 60 94 26 94 75
    twiddle BAROH1 64 128
    makelump BAROH5 63 91 32 91 77
    twiddle BAROH5 64 128
    makelump BAROH4H6 66 97 42 97 79
    twiddle BAROH4H6 128 128
    makelump BAROH3H7 58 95 27 95 81
    twiddle BAROH3H7 64 128
    makelump BAROH2H8 60 95 28 95 83
    twiddle BAROH2H8 64 128
    makelump BAROI0 65 91 33 91 85
    twiddle BAROI0 128 128
    makelump BAROJ0 64 82 32 82 87
    twiddle BAROJ0 64 128
    makelump BAROK0 58 74 28 74 89
    twiddle BAROK0 64 128
    makelump BAROL0 54 61 26 61 91
    twiddle BAROL0 64 64
    makelump BAROM0 54 48 27 48 93
    twiddle BAROM0 64 64
    makelump BARON0 58 34 29 33 95
    twiddle BARON0 64 64

You will now have a bunch of `.raw`, `.lmp` and `.twid` files.
The files of interest are the `.twid` files.

You will now need to compress the `.twid` files in order to get the sprites into the form needed for the WAD files.

    cd ~/doom64-dc/d64dcprep/monster
    for i in *.twid; do encode $i; done

This will result in a bunch of `.twid.enc` files. These are the final result of all of the transformations being done.

Next, copy the `.twid` AND `.twid.enc` files to the `~/doom64-dc/d64dcprep/wadfiles` directory.

    cp *.twid* ~/doom64-dc/d64dcprep/wadfiles

and change directory to the `~/doom64-dc/d64dcprep/wadfiles` directory and run the fixwad tool we built a few steps back

This reads `doom64.wad`. replaces the monster sprites with our newly re-colored, twiddled and compressed sprite lumps, and writes out a file named `out.wad` . 
I don't want to keep changing code so you'll need to update some permissions to access the file. Then rename that to `pow2.wad` and move that to `doom64-dc/selfboot` .

    cd ~/doom64-dc/d64dcprep/wadfiles
    fixwad
	chmod u+rw out.wad
    mv out.wad ~/doom64-dc/d64dcprep/selfboot/pow2.wad

One step remains. We need to make a new WAD file containing just the sprites with alternate palettes applied. From the wadfiles directory, run the `makewad` tool.  This creates a file named `alt.wad`. It has two marker lumps (`S2_START` / `S2_END`) and compressed graphic lumps in between.
I don't want to keep changing code so you'll need to update some permissions to access the file. Move that file to `~/doom64-dc/d64dcprep/selfboot`.

    cd ~/doom64-dc/d64dcprep/wadfiles
    makewad
	chmod u+rw alt.wad
    mv alt.wad ~/doom64-dc/d64dcprep/selfboot

Check the md5sum of each new wad file to see if they are as expected:

    md5sum ~/doom64-dc/d64dcprep/selfboot/pow2.wad 
    376fed2a0b00257de775e0815e0b3c5f  pow2.wad

    md5sum ~/doom64-dc/d64dcprep/selfboot/alt.wad
    2649f740d0dc2d930a219a1bde951d91  alt.wad


You now have all of the updated files required to run Doom 64 for Dreamcast in the places they need to be.

Go to the repo source directory and compile it like any other KallistiOS project. 

    cd ~/doom64-dc/src
    make
    sh-elf-objcopy -R .stack -O binary doom64.elf doom64.bin
    /opt/toolchains/dc/kos/utils/scramble/scramble doom64.bin ~/doom64-dc/selfboot/1ST_READ.BIN

Finally, make a self-booting CDI from the contents of the selfboot directory. 
I am not explaining this process. It depends on the tools and OS you are using.

Good luck.
