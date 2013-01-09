#include "..\include\llsetcontour.h"
#include <string.h>
#include <stdio.h>

//constructor
llSetContour::llSetContour() : llSet() {

	SetCommandName("ContourLine");

	findmin = findmax = linear = onlyintracell = setmin = setmax = 0;
}

int llSetContour::RegisterOptions(void) {
	if (!llSet::RegisterOptions()) return 0;

	RegisterValue("-x",       &gridx, LLWORKER_OBL_OPTION);
	RegisterValue("-y",       &gridy, LLWORKER_OBL_OPTION);
	RegisterValue("-z",       &z,     LLWORKER_OBL_OPTION);
	RegisterValue("-offsetx", &offsetx);
	RegisterValue("-offsety", &offsety);

	RegisterFlag("-findmin",       &findmin);
	RegisterFlag("-findmax",       &findmax);
	RegisterFlag("-setmin",        &setmin);
	RegisterFlag("-setmax",        &setmax);
	RegisterFlag("-linear",        &linear);
	RegisterFlag("-onlyintracell", &onlyintracell);

	return 1;
}


int llSetContour::Init(void) {
	if (!llSet::Init()) return 0;

	float minab = (float) _llUtils()->GetValueF("_mindistance");
	float cellsize_x = (float) _llUtils()->GetValueF("_cellsize_x");
	float cellsize_y = (float) _llUtils()->GetValueF("_cellsize_y");

	if (onlyintracell) {
		if (!cellsize_x) {
			_llLogger()->WriteNextLine(-LOG_WARNING,"%s: _cellsize_x not defined, -onlyintracell disabled", command_name);
			onlyintracell = 0;
		}
		if (!cellsize_y) {
			_llLogger()->WriteNextLine(-LOG_WARNING,"%s: _cellsize_y not defined, -onlyintracell disabled", command_name);
			onlyintracell = 0;
		}
	}
	
	int con_points=0;

	for (float y=_llUtils()->y00+offsety+gridy; y<=(_llUtils()->y11); y+=gridy) {
		int lastflag = 0;
		float minx   = 0;
		float minz   = 999999;
		float maxx   = 0;
		float maxz   = -999999;
		for (float x=_llUtils()->x00+offsetx+gridx; x<=(_llUtils()->x11); x+=gridx) {
			if (lastflag==-1 && findmin) {
				for (float x1=x-gridx; x1<=x; x1++) {
					if (heightmap->GetZ(x1,y) < minz) {
						minz = heightmap->GetZ(x1, y);
						minx = x1;
						if (onlyintracell && int(x1/cellsize_x)*int(cellsize_x)==int(x1)) {
							lastflag = 0;
							minz     = -999999;
						}
					}
				}
			}
			if (lastflag==1 && findmax) {
				for (float x1=x-gridx; x1<=x; x1++) {
					if (heightmap->GetZ(x1,y) > maxz) {
						maxz = heightmap->GetZ(x1, y);
						maxx = x1;
						if (onlyintracell && int(x1/cellsize_x)*int(cellsize_x)==int(x1)) {
							lastflag = 0;
							minz     = -999999;
						}
					}
				}
			}


			if (((heightmap->GetZ(x,y) > z) && (heightmap->GetZ(x-gridx,y) < z))
				|| ((heightmap->GetZ(x,y) < z) && (heightmap->GetZ(x-gridx,y) > z))) {
					//breakline crossing
					float myx = x-gridx*(heightmap->GetZ(x,y)-z)/
						(heightmap->GetZ(x,y)
						- heightmap->GetZ(x-gridx,y));
					if (!linear) {
						myx = 0;
						float numx = 0;
						for (float x1=x-gridx; x1<=x; x1++) {
							if ((heightmap->GetZ(x1,y) < z && heightmap->GetZ(x1-minab,y) >= z)
								|| (heightmap->GetZ(x1,y) > z && heightmap->GetZ(x1-minab,y) <= z)) {
									myx+=x1;
									numx++;
							}
						}
						if (numx) {
							myx /= numx;
						} else {
							_llLogger()->WriteNextLine(LOG_WARNING,"%s: contour not found for x", command_name);
						}
					}
					int flag=1;
					if (heightmap->GetZ(x,y) < heightmap->GetZ(x-gridx,y)) 
						flag=-1;
					if (lastflag==-1 && flag==1 && minz<99998) { 
						if (points->GetMinDistance(minx,y) > minab) { //  && 
							if (points->GetMinDistanceGrid(minx,y,1) > minab) {
								//points->AddPoint(minx,y,heightmap->GetZ(minx,y));	
								points->AddPoint(minx, y, z);con_points++;	
								minz = 99999;
							}
						} else points->SetFlag(points->GetClosestPoint(minx,y), flag);
					}
					if (lastflag==1 && flag==-1 && maxz>-99998) { 
						if (points->GetMinDistance(maxx,y) > minab) { //  && 
							if (points->GetMinDistanceGrid(maxx,y,1) > minab) {
								//points->AddPoint(minx,y,heightmap->GetZ(minx,y));	
								points->AddPoint(maxx, y, z);
								con_points++;	
								maxz = -99999;
							}
						} else points->SetFlag(points->GetClosestPoint(maxx,y), flag);
					}
					//float minab_grid=-1;
					float minab_grid = minab;
					if (points->GetMinDistance(myx,y) > minab && points->GetMinDistanceGrid(myx,y,1) > minab_grid) {
						int p = points->AddPoint(myx, y, heightmap->GetZ(myx,y));	
						con_points++;
						points->SetFlag(p, flag);
						if (setmin && heightmap->GetZ(x-gridx,y) < z && points->GetMinDistance(x-gridx,y) > minab) {
							points->AddPoint(x-gridx, y, heightmap->GetZ(x-gridx,y)); 
							con_points++;
						}
						if (setmin && heightmap->GetZ(x,y) < z && points->GetMinDistance(x,y) > minab) {
							points->AddPoint(x, y, heightmap->GetZ(x,y)); 
							con_points++;
						}
						if (setmax && heightmap->GetZ(x-gridx,y) > z && points->GetMinDistance(x-gridx,y) > minab) {
							points->AddPoint(x-gridx, y, heightmap->GetZ(x-gridx,y)); 
							con_points++;
						}
						if (setmax && heightmap->GetZ(x,y) > z && points->GetMinDistance(x,y) > minab) {
							points->AddPoint(x,y,heightmap->GetZ(x,y)); 
							con_points++;
						}

					}
					lastflag=flag;
			}
		}
	}

	for (float x=_llUtils()->x00+offsetx+gridx; x<=(_llUtils()->x11); x+=gridx) {
		int   lastflag = 0;
		float miny = 0;
		float minz = 999999;
		float maxy = 0;
		float maxz = -999999;
		for (float y=_llUtils()->y00+offsety+gridy; y<=(_llUtils()->y11-1); y+=gridy) {

			if (lastflag==-1 && findmin) {
				for (float y1=y-gridy; y1<=y; y1++) {
					if (heightmap->GetZ(x,y1) < minz) {
						minz = heightmap->GetZ(x, y1);
						miny = y1;
						if (onlyintracell && int(y1/cellsize_y)*int(cellsize_y)==int(y1)) {
							lastflag = 0;
							minz     = -999999;
						}
					}
				}
			}
			if (lastflag==1 && findmax) {
				for (float y1=y-gridy; y1<=y; y1++) {
					if (heightmap->GetZ(x,y1) > maxz) {
						maxz = heightmap->GetZ(x, y1);
						maxy = y1;
						if (onlyintracell && int(y1/cellsize_y)*int(cellsize_y)==int(y1)) {
							lastflag = 0;
							minz     = -999999;
						}
					}
				}
			}

			if (((heightmap->GetZ(x,y) > z ) && (heightmap->GetZ(x,y-gridy) < z))
				|| ((heightmap->GetZ(x,y) < z ) && (heightmap->GetZ(x,y-gridy) > z))) {
					//breakline crossing
					float myy = y-gridy*(heightmap->GetZ(x,y)-z)/
						(heightmap->GetZ(x,y)
						- heightmap->GetZ(x,y-gridy));
					if (!linear) {
						myy = 0;
						float numy = 0;
						for (float y1=y-gridy; y1<=y; y1++) {
							if ((heightmap->GetZ(x,y1) < z && heightmap->GetZ(x,y1-minab) >= z)
								|| (heightmap->GetZ(x,y1) > z && heightmap->GetZ(x,y1-minab) <= z)) {
									myy += y1;
									numy++;
							}
						}
						if (numy) {
							myy /= numy;
						} else {
							_llLogger()->WriteNextLine(LOG_WARNING,"%s: contour not found for y", command_name);
						}
					}
					int flag = 1;
					if (heightmap->GetZ(x,y) < heightmap->GetZ(x,y-gridy)) 
						flag=-1;
					if (lastflag==-1 && flag==1 && minz<99998) { 
						if (points->GetMinDistance(x,miny) > minab) { 
							if (points->GetMinDistanceGrid(x,miny,2) > minab) {
								//points->AddPoint(x,miny,heightmap->GetZ(x,miny));	
								points->AddPoint(x, miny, z);
								con_points++;	
								minz = 99999;
							}
						} else points->SetFlag(points->GetClosestPoint(x,miny), flag);
					}
					if (lastflag==1 && flag==-1 && maxz>-99998) { 
						if (points->GetMinDistance(x,maxy) > minab) { 
							if (points->GetMinDistanceGrid(x,maxy,2) > minab) {
								//points->AddPoint(x,miny,heightmap->GetZ(x,miny));	
								points->AddPoint(x, maxy, z);
								con_points++;	
								minz = 99999;
							}
						} else points->SetFlag(points->GetClosestPoint(x,maxy), flag);
					}
					//float minab_grid=-1;
					float minab_grid = minab;
					if (points->GetMinDistance(x,myy) > minab && points->GetMinDistanceGrid(x,myy,2) > minab_grid) {
						int p = points->AddPoint(x,myy,heightmap->GetZ(x,myy));	
						con_points++;
						points->SetFlag(p, flag);
						if (setmin && heightmap->GetZ(x,y-gridy) < z && points->GetMinDistance(x,y-gridy) > minab) {
							points->AddPoint(x, y-gridy, heightmap->GetZ(x,y-gridy)); 
							con_points++;
						}
						if (setmin && heightmap->GetZ(x,y) < z && points->GetMinDistance(x,y) > minab) {
							points->AddPoint(x, y, heightmap->GetZ(x,y)); 
							con_points++;
						}
						if (setmax && heightmap->GetZ(x,y-gridy)>z && points->GetMinDistance(x,y-gridy) > minab) {
							points->AddPoint(x, y-gridy, heightmap->GetZ(x,y-gridy)); 
							con_points++;
						}
						if (setmax && heightmap->GetZ(x,y) > z && points->GetMinDistance(x,y) > minab) {
							points->AddPoint(x, y, heightmap->GetZ(x,y)); 
							con_points++;
						}

					}
					lastflag=flag;
			}
		}
	}

	_llLogger()->WriteNextLine(-LOG_COMMAND,"%s: %i vertices set", command_name, con_points);


	return 1;
}