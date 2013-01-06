#include "..\include\llmap.h"

//constructor
llMap::llMap(unsigned int _x, unsigned int _y, int _makeshort, float _default) {

	widthx   = _x;
	widthy   = _y;
	f_widthx = float(_x);
	f_widthy = float(_y);
	mesg     = _llLogger();
	sdata    = NULL;
	idata    = NULL;

	makeshort = _makeshort;
	if (makeshort == 0 || makeshort == 1)
		sdata = new llShortarray(widthx*widthy, makeshort, _default); 
	else
		idata = new unsigned int[widthx*widthy];

	scaling=1;
	uneven=1;
	defaultheight = _default;

	InitRnd(0, 0, widthx-1, widthy-1);
}

llMap::llMap(unsigned int _x, unsigned int _y, llShortarray *_data, float _default) {

	widthx   = _x;
	widthy   = _y;
	f_widthx = float(_x);
	f_widthy = float(_y);
	mesg     = _llLogger();

	makeshort = 0;
	sdata = _data;
	idata    = NULL;

	scaling=1;
	uneven=1;
	defaultheight = _default;

	InitRnd(0, 0, widthx-1, widthy-1);

}

llMap * llMap::Clone(int _expand, int _makeshort) {
	llMap *dummy = new llMap(widthx*_expand, widthy*_expand, makeshort, defaultheight);
	dummy->SetCoordSystem(x1, y1, x2, y2, z);
	return dummy;
}

llMap::~llMap() {
    if (sdata) delete sdata; 
	if (idata) delete idata; 
}

void llMap::InitRnd(unsigned int _x1, unsigned int _y1, unsigned int _x2, unsigned int _y2) {
	rnd_x1 = _x1;
	rnd_x2 = _x2;
	rnd_y1 = _y1;
	rnd_y2 = _y2;
	rnd_widthx = _x2 - _x1;
	rnd_widthy = _y2 - _y1;

	rndx = rnd_widthx;
	rndy = rnd_widthy;

	bool flagx = false;
	bool flagy = false;
	for (int i=31; i>=0; i--) {
		if (!flagx) {
			unsigned int pax = (rndx >> i) & 0x1;
			if (pax) flagx = true;
		} else
			rndx |= (0x1 << i);
		if (!flagy) {
			unsigned int pay = (rndy >> i) & 0x1;
			if (pay) flagy = true;
		} else
			rndy |= (0x1 << i);
	}
}

unsigned int llMap::GetRndX() {
loop:
	unsigned int rnd = rand() & rndx; //BUGBUG: works only up to RAND_MAX
	if (rnd > rnd_widthx) goto loop;
	return rnd;
}

unsigned int llMap::GetRndY() {
loop:
	unsigned int rnd = rand() & rndy;
	if (rnd > rnd_widthy) goto loop;
	return rnd;
}