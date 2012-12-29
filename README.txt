Overview
========
This tool creates LOD meshes based on the landscape heightmap. Therefore, it can be used to replace the meshes of Oblivion including all the additional changes added by the mods. 

Moreover, a configuration file (multi purpose batch file) with a command list allows to control the behavior of vertex setting and calculation completely, up to the placement of single vertices.

Author
======
Ingo Froehlich aka. gruftikus@nexusmods

Why?
====
In the Oblivion game engine, the LOD meshes can be replaced quad-by-quad only. If one has a lot of landscape changes, the following problems might appear:

* The quad meshes do not fit at their borders, because they were created independently.

* "Wall-like" steps between the normal grid and the LOD meshes.

* Floating objects (trees, buildings).

Features
========
1. To generate the LODs, tes4ll triangulates the entire worldspace in one single step. The single quads are created and written based on this complete world mesh. This avoids ugly "steps" in the far landscape.

2. tes4ll uses an irregular mesh pattern, whereas the edges of the triangles from the Construction Set are oriented in multiples of 45 degrees.

3. tes4ll is completely configurable by the user by adapting a script (called multi-purpose batch file):

  * Single vertex points can be self-defined to avoid gaps, floating trees, etc., wherever you want.

  * Data files can be used with hand-placed vertex points. One can also define break lines or polygons.

  * Contour lines can be added

  * Several algorithms can be used to define individual vertex points density functions:

    - First and second order derivatives (i.e. based on the slope and the curvature of the heightmap)

    - A radial density

    - A peak finder for better mountains

    - Enhanced/reduced density based on the absolute height

  * The number of vertices per quad can be chosen. Depending on your system capabilities, a larger or lower number of vertices can be defined.

4. For debugging and better visibility nif files with textures and ps (postscript) files can be produced.

5. Tes4ll can (optionally) also call Lightwaves TES4qLOD(http://projectmanager.f2s.com/morrowind/TES4qLOD.html), so that the meshes are always consistent with the texture set. Since tes4ll v4, TES4qLOD has been fully integrated.

6. Tes4ll can produce normal maps, also with the "faked terrain shadow" option.


Details of operation
====================
The main idea of the program is based on different steps which are executed one after the other (see script example, for a detailed description of the commands see MANUAL.txt).

In the FIRST STEP, deterministic vertex points are set by commands. The most simple case is the placement of single vertices. Other examples are the contour setter or the panorama view.

In the SECOND STEP, random vertex points are produced and compared to density functions by using the Monte-Carlo method (see link below for more explanation). This works (explained only sketchy) as follows: the program scatters a lot of random (x,y) points on the heightmap. For each of this points another random number is generated between 0 and the maximum of the density (the ceiling). If the random number if below the local density, the vertex point is taken to the final list, otherwise it is re-sampled. As a consequence, the density of the vertex points in the final list represents the value of the density function. One should note that, this density function can be defined by the user by combining the basic algorithms. The distance to the next neightbor is taken into account to avoid local clustering.

The ceiling has a certain cut-off, to avoid infinity computing times.

In the THIRD STEP the vertex points are triangulated, i.e. the points are connected to triangles. The usual method to do so is the Delaunay triangulation.

In the FORTH STEP, the mesh is cut into cells. This avoids ugly steps between the near cells and the LOD meshes. Moreover, the triangles can be further split by using special commands (like polygons or simple lines).

In STEP 5, the mesh is furthermore divided into the final LOD meshed (the "quads").

In STEP 6, the triangles are written and (optionally) stitched together in triangle strips (see links below). 

The syntax of the batch file
============================
A full reference manual can be obtained via this link:
https://github.com/Gruftikus/tes4ll/blob/master/REFERENCE_MANUAL-tes4ll.pdf?raw=true

Source code
===========
The source code should be freely available from Github (https://github.com/Gruftikus/tes4ll.git). You can check out the repository and compile tes4ll from scratch. In order to do so, the following steps have to be done in addition:

Make a directory "externals" in tes4ll.

Download niflib, and unpack its source code to tes4ll/externals/niflib.

Download "triangle" from http://www.cs.cmu.edu/~quake/triangle.html, and unpack the source code to tes4ll/externals/triangle. You can also choose a Delaunay code of your choice, in this case you have to adapt the section MAKE_TRIANGULATION in tes4ll.cpp

Download zlib, unpack the source code to tes4ll/externals/zlib.

Download "dirent.h" from somewhere (just search the internet), and put it to tes4ll/externals/.

Disclaimer: tes4ll started as a private project, and was later merged with other code. Therefore it contains c++ classes, but also a lot of procedural stuff. Most of the things have been done quick and dirty. You have been warned!


Version and changelog
=====================
Version 1:    Initial version.

Version 1.01: 3GB flag for larger worldspaces, 
              Crash fixed which appeared when nif meshes were empty.

Version 1.10: Added option for the pedestals.
              Re-calibrated the x,y-position of the bitmap.
              Removed distance keeper for the contour to the grid.

Version 1.20: Added option for the data files.
              First data files for some segments (north of Weye, 
              way to Cheydinhal)
              Data file for UL Rolling Hills.

Version 1.21: Fixed bug in SetMaxPointsPerQuad.

Version 2.00: Support for different wordspaces.
              Call TESAnnwyn now internally.
              Support for flags.

Version 2.10: Added option for tri shapes.

Version 2.20: Added wall remover (BreakAtGrid).

Version 2.30: Decreased memory usage by a factor of 2.
              Decoupled "hardcoded" TESAnnwyn and moved its call to 
              the batch file. Added TES4qLOD call to the list of 
              commands.

Version 3.00: Changed a lot here and there to make tes4ll compatible
              with MPGUI.

Version 3.01: Fixed missing quotation marks stripping in 
              -installdirectory.

Version 3.10: First release of the normal map version.

Version 4.00: Integration of Lightwaves TES4qLOD, and partly
              rewritten tes4ll. Got rid of the TESAnnwyn dependency.

Version 4.10: Added the feature to define polygons, and to stencil
              polygons. Added break lines. Made a lot of code cleanup.
              Improved the speed of the vertex placement by a factor
              of 5-10. Added a remover for hidden triangles.
              

Literature
==========
* http://en.wikipedia.org/wiki/Monte_Carlo_method

* P. Pourke, "Triangulate: Efficient Triangulation Algorithm Suitable for Terrain Modelling", Presented at Pan Pacific Computer Conference, Beijing, China. January 1989, see link: http://paulbourke.net/papers/triangulate/

* A. James Stewart, "Tunneling for Triangle Strips in Continuous "Level-of-Detail Meshes", Published in: Proceedings GRIN'01

* M. B. Porcu, and R. Scateni, "An Iterative Stripification Algorithm Based on Dual Graph Operations", EUROGRAPHICS 2003

* M. B. Porcu, N. Sanna and R. Scateni, "Efficiently keeping an optimal stripification over a CLOD mesh", The Journal of WSCG, Vol.13

* J. R. Shewchuk, "Triangle: Engineering a 2D Quality Mesh Generator and Delaunay Triangulator", in "Applied Computational Geometry: Towards Geometric Engineering" (Ming C. Lin and Dinesh Manocha, editors), volume 1148 of Lecture Notes in Computer Science, pages 203-222, Springer-Verlag, Berlin, May 1996.

Credits
=======
This program uses the following 3rd party source code:

* niflib: http://niftools.sourceforge.net/wiki/NifSkope
* bmp file reader: written by P. Pourke
* Triangle: by J. R. Shewchuk (see reference above)
* TES4qLOD by Lightwave (taken from http://projectmanager.f2s.com/morrowind/TES4qLOD.html)

License
=======
Copyright (C) 2011-2012

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