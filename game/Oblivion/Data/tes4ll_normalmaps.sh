#!/bin/bash
export WINEPREFIX=$(echo $PWD | grep 'drive_c' | sed 's/drive_c.*//g')
wine ./tes4ll.exe makenormalmaps overwritelods z_val=1.0 logfile=tes4ll_normalmaps.log c_dxt5 ini/tes4ll/tes4ll_all.mpb

