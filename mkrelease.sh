#!/bin/sh

#mkfw path
#export PATH=~/mkfw:$PATH

cd esplay-launcher
ffmpeg -i main/gfxTile.png -f rawvideo -pix_fmt rgb565 main/gfxTile.raw -y
cat main/gfxTile.raw | xxd -i > main/gfxTile.inc
#idf.py menuconfig
idf.py build
cd ../esplay-gnuboy
#idf.py menuconfig
idf.py build
cd ../esplay-nofrendo
#idf.py menuconfig
idf.py build
cd ../esplay-smsplusgx
#idf.py menuconfig
idf.py build
cd ..

../mkfw Retro-Emulation assets/tile.raw 0 16 1310720 launcher esplay-launcher/build/esplay-launcher.bin 0 17 524288 esplay-nofrendo esplay-nofrendo/build/esplay-nofrendo.bin 0 18 458752 esplay-gnuboy esplay-gnuboy/build/esplay-gnuboy.bin 0 19 1179648 esplay-smsplusgx esplay-smsplusgx/build/esplay-smsplusgx.bin
rm esplay-retro-emu.fw
mv firmware.fw esplay-retro-emu.fw
