#include "..\include\llsplitatgrid.h"
#include <string.h>
#include <stdio.h>

//constructor
llSplitAtGrid::llSplitAtGrid() : llSet() {

	SetCommandName("SplitAtGrid");
}

int llSplitAtGrid::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x",    &gridx, LLWORKER_OBL_OPTION);
	RegisterValue("-y",    &gridy, LLWORKER_OBL_OPTION);
	RegisterValue("-zmin", &zmin);
	RegisterValue("-max",  &max,   LLWORKER_OBL_OPTION);

	return 1;
}


int llSplitAtGrid::Init(void) {
	if (!llSet::Init()) return 0;
	float minab=256;

	int zused = 0;
	if (!Used("-zmin")) zused = 1;

	int gb_points=0;

	for (float x=floor(_llUtils()->x00/gridx)*gridx; x<=(_llUtils()->x11)-gridx; x+=gridx) {
		for (float y=floor(_llUtils()->y00/gridy)*gridy; y<=(_llUtils()->y11)-gridy; y+=gridy) {

			//List of segments
			float segstart[1000];
			float segend[1000];
			int segaktive[1000];
			int segpointer=0;

			//fill the first segment
			segstart[segpointer]  = x;
			segend[segpointer]    = x + gridx;
			segaktive[segpointer] = 1;
			segpointer++;

			for (int i=0; i<segpointer; i++) {

				//is segment large enough?
				if (segend[i]-segstart[i]>2*minab && segaktive[i]) {
					float mymax = -1;
					float myx   = segstart[i]+minab;
					float z     = heightmap->GetZ(segstart[i]+minab,y);
					float slope = (heightmap->GetZ(segend[i]-minab,y) -z ) / (segend[i] - segstart[i] - 2*minab);

					for (float x1 = segstart[i]+minab ;x1 < segend[i]-minab; x1++) {
						float walldiff = (z + slope * (x1 - (segstart[i]+minab))) - heightmap->GetZ(x1,y);
						if (walldiff > max && walldiff>mymax && (heightmap->GetZ(x1,y)>zmin || zused)) {
							mymax = walldiff;
							myx   = x1;
						}
					}

					if (mymax>0) {
						//Split segment
						points->AddPoint(myx,y,heightmap->GetZ(myx,y));	
						//gen_npoints++;
						gb_points++;
						segaktive[i]=0;
						segstart[segpointer]  = segstart[i];
						segend[segpointer]    = myx;
						segaktive[segpointer] = 1;
						segpointer++;
						segstart[segpointer]  = myx;
						segend[segpointer]    = segend[i];
						segaktive[segpointer] = 1;
						segpointer++;
						i = 0;
					} else segaktive[i] = 0;

				} else segaktive[i] = 0;
			}

			segpointer=0;
			//fill the first segment
			segstart[segpointer]  = y;
			segend[segpointer]    = y + gridy;
			segaktive[segpointer] = 1;
			segpointer++;

			for (int i=0; i<segpointer; i++) {

				//is segment large enough?
				if (segend[i]-segstart[i]>2*minab && segaktive[i]) {
					float mymax=-1;
					float myy=segstart[i]+minab;
					float z=heightmap->GetZ(x,segstart[i]+minab);
					float slope=(heightmap->GetZ(x,segend[i]-minab) -z ) / (segend[i] - segstart[i] - 2*minab);

					for (float y1 = segstart[i]+minab ;y1 < segend[i]-minab; y1++) {
						float walldiff = (z + slope * (y1 - (segstart[i]+minab))) - heightmap->GetZ(x,y1);
						if (walldiff > max && walldiff>mymax && (heightmap->GetZ(x,y1)>zmin || zused)) {
							mymax = walldiff;
							myy   = y1;
						}
					}

					if (mymax>0) {
						//Split segment
						points->AddPoint(x,myy,heightmap->GetZ(x,myy));	
						//gen_npoints++;
						gb_points++;
						segaktive[i]          = 0;
						segstart[segpointer]  = segstart[i];
						segend[segpointer]    = myy;
						segaktive[segpointer] = 1;
						segpointer++;
						segstart[segpointer]  = myy;
						segend[segpointer]    = segend[i];
						segaktive[segpointer] = 1;
						segpointer++;
						i = 0;
					} else segaktive[i] = 0;

				} else segaktive[i] = 0;
			}
#if 0
			if (points->GetMinDistance(x1,y1) > minab) {
				points->AddPoint(x1,y1,heightmap->GetZ(x1,y1));	
				gen_npoints++;
			}
#endif
		}
	}
	
	_llLogger()->WriteNextLine(-LOG_INFO,"%i break vertices set", gb_points);


	return 1;
}