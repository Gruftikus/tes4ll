#!/bin/bash
export WINEPREFIX=$(echo $PWD | grep 'drive_c' | sed 's/drive_c.*//g')
wine ./tes4ll.exe usedatafiles makemeshes mid_res wallremover contour nicer_mountains overwritelods logfile=tes4ll.log ini/tes4ll/tes4ll_all.mpb

