#!/bin/bash
export WINEPREFIX=$(echo $PWD | grep 'drive_c' | sed 's/drive_c.*//g')
wine ./tes4ll.exe tes4qlod qlod1 option_blending overwritelods logfile=tes4qlod.log ini/tes4ll/tes4ll_all.mpb

