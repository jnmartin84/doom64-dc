# doom64-dc
Doom 64 port for Sega Dreamcast / KallistiOS 

# Pre-reqs:

Install `SLADE` (for Linux see https://flathub.org/apps/net.mancubus.SLADE)
Install `Doom64 EX` / `Doom64 EX+` (idk, you're on your own for this)

Install `rename` if you don't already have it
( `sudo apt-get install rename` )

Install `python3` and `python3-pip` if you don't already have them

( `sudo apt-get install python3 python3-pip ` )

Pip install Pillow
`pip3 install pillow`

Install `GIMP 2.x` 
( `sudo apt-get install gimp` )

In my Ubuntu VM this got me GIMP 2.10 so I will refer to it by version number later.
You may have to change it.

Install some `Qt5` dependencies for `texconv`
sudo apt install qt5-qmake qtbase5-dev


# How to generate sprite textures:

A subdirectory exists in the repo to use for placing all the files needed to make a bootable disc image
referred to hereon out as repo/selfboot .

A subdirectory exists in the repo to use for all of the texture preparation
referred to as repo/d64dcprep

Check that your doom64 ROM is the correct one

The below is the expected md5sum output

`$ md5sum.exe doom64.z64`
`b67748b64a2cc7efd2f3ad4504561e0e *doom64.z64`

dump the original N64 Doom 64 IWAD from the ROM
`dd if=doom64.z64 of=repo/selfboot/doom64.wad bs=1 skip=408848 count=6101168`

Run Doom64ex's WADGEN against doom64.z64 to dump DOOM64.WAD.

open the WADGEN DOOM64.WAD in SLADE

in SLADE, select lumps 1 through 346,
right-click, click on Export
select the repo/d64dcprep/nonenemy directory and click Save
select lumps 906 through 947
right-click, click on Export
select the repo/d64dcprep/nonenemy directory and click Save

next, in SLADE, select lumps 347 through 905 
right-click, click on Export
select the repo/d64dcprep/monster directory and click Save

In a terminal

cp repo/d64dcprep
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
cd tmp
gimp --verbose -i -b '(doom64-alt-monster "BARO*.png" "palbaro.png")' -b '(gimp-quit 0)'
gimp --verbose -i -b '(doom64-alt-monster "ZOMB*.png" "palzomb.png")' -b '(gimp-quit 0)'
gimp --verbose -i -b '(doom64-alt-monster "SPEC*.png" "palspec.png")' -b '(gimp-quit 0)'
gimp --verbose -i -b '(doom64-alt-monster "NITE*.png" "palnite.png")' -b '(gimp-quit 0)'
mv BARO*.png ..
mv NITE*.png ..
mv SPEC*.png ..
mv ZOMB*.png ..
cd ..
 
gimp --verbose -i -b '(batch-doom64-colorconvert "*.png" "doom64monster")' -b '(gimp-quit 0)' 

cd ..
cd nonenemy
mv ARM*.png tmp
gimp --verbose -i -b '(batch-doom64-colorconvert "*.png" "doom64nonenemy")' -b '(gimp-quit 0)'    
cd tmp
gimp --verbose -i -b '(batch-doom64-colorconvert-nodither "ARM*.png" "doom64nonenemy")' -b '(gimp-quit 0)'   
mv ARM*.png ..   
cd ..   
cd ..
    
now that the individual graphics have been reindexed and flattened, it is time to make spritesheet textures out of them
cd repo/d64dcprep
python3 sheeter.py

The output will be 11 png files in the `repo/d64dcprep/sheets` subdirectory

You now need to clone the `texconv` github repo, apply a patch to it
and build it.

cd repo
git clone https://github.com/tvspelsfreak/texconv.git
mv texconv_doom64dc.patch texconv
cd texconv
git apply texconv_doom64dc.patch
qmake
make

Next, add the texconv directory to your path to make the next steps easier

export PATH=repo/texconv:$PATH
cd repo/d64dcprep/sheets

texconv --in barniteshot.png --out barniteshot.vq --format PAL8BPP --compress
texconv --in fattrect1.png --out fattrect1.vq --format PAL8BPP --compress
texconv --in fattrect2.png --out fattrect2.vq --format PAL8BPP --compress
texconv --in fattrect3.png --out fattrect3.vq --format PAL8BPP --compress
texconv --in painbsp.png --out painbsp.vq --format PAL8BPP --compress
texconv --in playtrooposs.png --out playtrooposs.vq --format PAL8BPP --compress
texconv --in remcybr.png --out remcybr.vq --format PAL8BPP --compress
texconv --in sargfirstcybr.png --out sargfirstcybr.vq --format PAL8BPP --compress
texconv --in skulbosshead.png --out skulbosshead.vq --format PAL8BPP --compress
texconv --in spectre.png --out spectre.vq --format PAL8BPP --compress
texconv --in non_enemy.png --out non_enemy.tex --format PAL8BPP --nonenemy

rm *.vq.pal
rm *.tex.pal
mv *.vq repo/selfboot/vq
mv *.tex repo/selfboot/vq

