#!/bin/bash
export WINEPREFIX=$(echo $PWD | grep 'drive_c' | sed 's/drive_c.*//g')
wine ./tes4ll.exe tes4qlod_cache logfile=tes4qlod_cache.log ini/tes4ll/tes4ll_all.mpb
