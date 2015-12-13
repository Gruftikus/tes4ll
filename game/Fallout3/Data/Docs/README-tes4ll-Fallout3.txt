Important:

!!! Before your start, please make a backup of your meshes in Data\meshes\landscape\lod, and Data\textures\landscape\lod\


Requirements
============
None. If you need a GUI, use MPGUI (http://oblivion.nexusmods.com/mods/41447).


Installation
============
Unpack the tes4ll 7z-archive and move the content into the data directory of Fallout 3. When asked, merge the ini-directory of tes4ll into your ini.

After this procedure you should have one executable in your data directory (tes4ll.exe), several windows bat-files (*.bat), and the bitmap cache (for Vanilla textures) files which came with TES4qLOD.


Uninstallation
==============
Restore the old LOD meshes in Data\meshes\landscape\lod and the textures in Data\textures\landscape\lod\ which you (hopefully!) saved. Delete tes4ll.exe, and the content of ini\tes4ll


Generation of meshes (the simple method)
========================================
Actually, there are 2 simple methods:

1. Start the one of the Windows-bat-files by double clicking on it. 

At the moment, 3 versions are supported:

"tes4ll_make_normal_meshes.bat"  -> "Normal resolution", similar resolution as Vanilla LOD files.
"tes4ll_make_highres_meshes.bat" -> "High resolution", 50% more resolution
"tes4ll_make_ultimate_meshes.bat" -> "Ultimate resolution", 100% more resolution

The generation can take some time. Please have some patience.

2. Tes4ll is compatible to MPGUI. In order to use the GUI, you first must install MPGUI. After installation, you can either double-click on one of the *.mpb files in ini\tes4ll, or start MPGUI and open one of the files. With MPGUI you can save you complete configuration of you want. For details of operation, please open the help documents of MPGUI.


Normalmaps
==========
Tes4ll can also produce normalmaps. Use tes4ll_make_normalmaps.bat (or MPGUI with tes4ll_all.mpb) for this purpose. 


TES4qLOD
========
TES4qLOD uses some 4x4 bitmaps ("cache") which are located in Data\tes4qlod_tex, and are generated mainly for Vanilla textures. It is possible to re-generate the entire cache with "tes4ll_make_texture_cache.bat", and it should be done if you have texture replacers installed. Don't be surprised, the output in the shell is quite long. Texture caching has to be redone only in case of new installations of texture mods.

After this step one has to use tes4ll_make_colormaps.bat. 

Both of the above-mentioned steps can be done with MPGUI and a double-click on tes4ll_all.mpb much more elegant.


Generation of meshes (expert modus)
===================================
Adapt the configuration file for your needs. A detailed manual of the commands can be found at github:

* For general LOD (game-independent) commands: https://github.com/Gruftikus/lltool/wiki

* For TES-specific commands: https://github.com/Gruftikus/tes4ll/wiki

There are many things possible, tes4ll is highly adaptable. The details of this goes beyond this README. If you are in trouble, please contact me.


License
=======
Copyright (C) 2011-2015
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
