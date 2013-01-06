#include "..\include\lldiamondsquare.h"
#include "..\include\llmaplist.h"
#include "..\include\llalglist.h"

//constructor
llDiamondSquare::llDiamondSquare() : llWorker() {

	SetCommandName("DiamondSquare");
	mapname  = NULL;
	alg_list = NULL;
	steps    = 1;
	offset   = 0.;

}

int llDiamondSquare::RegisterOptions(void) {
	if (!llWorker::RegisterOptions()) return 0;

	RegisterValue("-steps",   &steps);
	RegisterValue("-mapname", &mapname);
	RegisterValue("-alg",     &alg_list);
	RegisterValue("-offset",  &offset);
	RegisterValue("-range",   &range, LLWORKER_OBL_OPTION);

	return 1;
}

int llDiamondSquare::Init(void) {
	if (!llWorker::Init()) return 0;

	cout << "ds" << endl;

	llMap *oldmap = NULL;
	if (!Used("-mapname"))
		mapname = "_heightmap";

	llAlgCollection *algs = NULL;
	if (Used("-alg")) {
		algs = _llAlgList()->GetAlgCollection(alg_list);
		if (!algs) {
			_llLogger()->WriteNextLine(-LOG_FATAL, "%s: alg collection '%s' not found", command_name, alg_list);
			return 0;
		}
	}

//	float range_alg = range;
	double alg = 1.f;
	offset = 0.5 - offset;

	oldmap = _llMapList()->GetMap(mapname);

	if (!oldmap) {
		_llLogger()->WriteNextLine(-LOG_ERROR,"%s: map '%s' not found", command_name, mapname);
		return 0;
	}

	unsigned int old_widthx = oldmap->GetWidthX();
	unsigned int old_widthy = oldmap->GetWidthY();

	unsigned int new_widthx = (old_widthx - 1) * 2 + 1;
	unsigned int new_widthy = (old_widthy - 1) * 2 + 1;

	llMap * heightmap = new llMap(new_widthx, new_widthy);
	heightmap->SetCoordSystem(oldmap->GetX1(), oldmap->GetY1(), oldmap->GetX2(), oldmap->GetY2(), oldmap->GetZScale());

	//Expansion step
	for (unsigned int x=0; x<old_widthx; x++) {
		for (unsigned int y=0; y<old_widthy; y++) {
			heightmap->SetElementRaw(2*x, 2*y, oldmap->GetElementRaw(x,y));
		}
	}

	//Mean step
	for (unsigned int x=0; x<old_widthx-1; x++) {
		for (unsigned int y=0; y<old_widthy-1; y++) {
			alg = 1.0;
			if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x+1), heightmap->GetCoordY(2*y+1));
			
			double alg00 = 1.0;
			if (algs) alg00 = 1. - fabs(algs->GetValue(oldmap->GetCoordX(x), oldmap->GetCoordY(y)) - alg);
			if (alg00 < 0.) alg00 = 0.;
			double alg10 = 1.0;
			if (algs) alg10 = 1. - fabs(algs->GetValue(oldmap->GetCoordX(x+1), oldmap->GetCoordY(y)) - alg);
			if (alg10 < 0.) alg10 = 0.;
			double alg01 = 1.0;
			if (algs) alg01 = 1. - fabs(algs->GetValue(oldmap->GetCoordX(x), oldmap->GetCoordY(y+1)) - alg);
			if (alg01 < 0.) alg01 = 0.;
			double alg11 = 1.0;
			if (algs) alg11 = 1. - fabs(algs->GetValue(oldmap->GetCoordX(x+1), oldmap->GetCoordY(y+1)) - alg);
			if (alg11 < 0.) alg11 = 0.;

			float mean;
			if (alg00+alg10+alg01+alg11)			
				mean = (alg00*oldmap->GetElementRaw(x,y)
				+ alg10*oldmap->GetElementRaw(x+1,y)
				+ alg01*oldmap->GetElementRaw(x,y+1)
				+ alg11*oldmap->GetElementRaw(x+1,y+1)) / (alg00+alg10+alg01+alg11);
			else
				mean = (oldmap->GetElementRaw(x,y)
				+ oldmap->GetElementRaw(x+1,y)
				+ oldmap->GetElementRaw(x,y+1)
				+ oldmap->GetElementRaw(x+1,y+1)) / 4.0;

t:
			mean += range*alg*(float(rand())/float(RAND_MAX) - offset);
			if (0 && oldmap->GetElementRaw(x,y) > mean &&
				oldmap->GetElementRaw(x+1,y) > mean &&
				oldmap->GetElementRaw(x,y+1) > mean &&
				oldmap->GetElementRaw(x+1,y+1) > mean) {
					offset -=0.1;
					goto t;
			}
			//cout << 2*x+1 << ":" << 2*y+1 << ":" << alg  << ":" << mean << endl;
			heightmap->SetElementRaw(2*x+1, 2*y+1, mean);
		}
	}
	
	offset = 0.5;

	//Diamond step
	for (unsigned int x=0; x<old_widthx; x++) {
		for (unsigned int y=0; y<old_widthy-1; y++) {
			if (x==0) {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(0), heightmap->GetCoordY(2*y+1));

				double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(0), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg02 = 1.0;
				if (algs) alg02 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(0), heightmap->GetCoordY(2*y+2)) - alg);
				if (alg02 < 0.) alg02 = 0.;

				float mean;
				if (alg+alg00+alg02)			
					mean = (alg00*heightmap->GetElementRaw(0,2*y)
					+ alg02*heightmap->GetElementRaw(0,2*y+2)
					+ alg*heightmap->GetElementRaw(1,2*y+1)) / (alg+alg00+alg02);
				else
					mean = (heightmap->GetElementRaw(0,2*y)
					+ heightmap->GetElementRaw(0,2*y+2)
					+ alg*heightmap->GetElementRaw(1,2*y+1)) / 3.0;
				//cout << 0 << ":" << 2*y+1 << ":" << alg << endl;
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				heightmap->SetElementRaw(0, 2*y+1, mean);
			} else if (x==(old_widthx-1)) {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y+1));

				double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg02 = 1.0;
				if (algs) alg02 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y+2)) - alg);
				if (alg02 < 0.) alg02 = 0.;

				float mean;
				if (alg+alg00+alg02)			
					mean = (alg00*heightmap->GetElementRaw(2*x,2*y)
					+ alg02*heightmap->GetElementRaw(2*x,2*y+2)
					+ alg*heightmap->GetElementRaw(2*x-1,2*y+1))/(alg+alg00+alg02);
				else
					mean = (heightmap->GetElementRaw(2*x,2*y)
					+ heightmap->GetElementRaw(2*x,2*y+2)
					+ heightmap->GetElementRaw(2*x-1,2*y+1))/(3.0);
				//cout << 2*x << ":" << 2*y+1 << ":" << alg << endl;
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				heightmap->SetElementRaw(2*x, 2*y+1, mean);
			} else {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y+1));

				double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg02 = 1.0;
				if (algs) alg02 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y+2)) - alg);
				if (alg02 < 0.) alg02 = 0.;

				float mean;
				if (alg+alg00+alg02)			
					mean = (alg00*heightmap->GetElementRaw(2*x,2*y)
					+ alg02*heightmap->GetElementRaw(2*x,2*y+2)
					+ alg*heightmap->GetElementRaw(2*x-1,2*y+1)
					+ alg*heightmap->GetElementRaw(2*x-1,2*y-1))/(alg00+alg02+alg+alg);
				else
					mean = (heightmap->GetElementRaw(2*x,2*y)
					+ heightmap->GetElementRaw(2*x,2*y+2)
					+ heightmap->GetElementRaw(2*x-1,2*y+1)
					+ heightmap->GetElementRaw(2*x-1,2*y-1))/(4.0);
				//cout << 2*x << ":" << 2*y+1 << ":" << alg << endl;
u:
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				if (0 && heightmap->GetElementRaw(2*x,2*y) > mean &&
					heightmap->GetElementRaw(2*x,2*y+2) > mean &&
					heightmap->GetElementRaw(2*x-1,2*y+1) > mean &&
					heightmap->GetElementRaw(2*x-1,2*y-1)) {
						offset -=0.1;
						goto u;
				}

				heightmap->SetElementRaw(2*x, 2*y+1, mean);
			} 
		}
	}

	for (unsigned int y=0; y<old_widthy; y++) {
		for (unsigned int x=0; x<old_widthx-1; x++) {
			if (y==0) {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x+1), heightmap->GetCoordY(0));

				double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg20 = 1.0;
				if (algs) alg20 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x+2), heightmap->GetCoordY(2*y)) - alg);
				if (alg20 < 0.) alg20 = 0.;

				float mean;				
				if (alg+alg00+alg20)			
					mean = (alg00*heightmap->GetElementRaw(2*x,0)
					+ alg20*heightmap->GetElementRaw(2*x+2,0)
					+ alg*heightmap->GetElementRaw(2*x+1,1))/(alg+alg00+alg20);
				else
					mean = (heightmap->GetElementRaw(2*x,0)
					+ heightmap->GetElementRaw(2*x+2,0)
					+ heightmap->GetElementRaw(2*x+1,1))/(3.0);
				//cout << 2*x+1 << ":" << 0 << ":" << alg << endl;
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				heightmap->SetElementRaw(2*x+1, 0, mean);
			} else if (y==(old_widthy-1)) {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x+1), heightmap->GetCoordY(2*y));

				double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg20 = 1.0;
				if (algs) alg20 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x+2), heightmap->GetCoordY(2*y)) - alg);
				if (alg20 < 0.) alg20 = 0.;

				float mean;
				if (alg+alg00+alg20)			
					mean = (alg00*heightmap->GetElementRaw(2*x,2*y)
					+ alg20*heightmap->GetElementRaw(2*x+2,2*y)
					+ alg*heightmap->GetElementRaw(2*x+1,2*y-1))/(alg+alg00+alg20);
				else
					mean = (heightmap->GetElementRaw(2*x,2*y)
					+ heightmap->GetElementRaw(2*x+2,2*y)
					+ heightmap->GetElementRaw(2*x+1,2*y-1))/(3.0);
				//cout << 2*x+1 << ":" << 2*y << ":" << alg << endl;
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				heightmap->SetElementRaw(2*x+1, 2*y, mean);
			} else {
				alg = 1.0;
				if (algs) alg = algs->GetValue(heightmap->GetCoordX(2*x+1), heightmap->GetCoordY(2*y));

								double alg00 = 1.0;
				if (algs) alg00 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x), heightmap->GetCoordY(2*y)) - alg);
				if (alg00 < 0.) alg00 = 0.;
				double alg20 = 1.0;
				if (algs) alg20 = 1. - fabs(algs->GetValue(heightmap->GetCoordX(2*x+2), heightmap->GetCoordY(2*y)) - alg);
				if (alg20 < 0.) alg20 = 0.;

				float mean;
				if (alg+alg00+alg20)			
					mean = (alg00*heightmap->GetElementRaw(2*x,2*y)
					+ alg20*heightmap->GetElementRaw(2*x+2,2*y)
					+ alg*heightmap->GetElementRaw(2*x+1,2*y-1)
					+ alg*heightmap->GetElementRaw(2*x+1,2*y+1))/(alg00+alg20+alg+alg);
				else
					mean = (heightmap->GetElementRaw(2*x,2*y)
					+ heightmap->GetElementRaw(2*x+2,2*y)
					+ heightmap->GetElementRaw(2*x+1,2*y-1)
					+ heightmap->GetElementRaw(2*x+1,2*y+1))/(4.0);
				//cout << 2*x+1 << ":" << 2*y << ":" << alg << endl;
v:
				mean += range*alg*(float(rand())/float(RAND_MAX)-offset);
				if (0 && heightmap->GetElementRaw(2*x,2*y) > mean &&
					+ heightmap->GetElementRaw(2*x+2,2*y) > mean &&
					+ heightmap->GetElementRaw(2*x+1,2*y-1) > mean &&
					+ heightmap->GetElementRaw(2*x+1,2*y+1) > mean) {
						offset -=0.1;
						goto v;
				}
				heightmap->SetElementRaw(2*x+1, 2*y, mean);
			} 
		}
	}

	delete (oldmap);
	_llMapList()->ExchangeMap(mapname, heightmap);

	return 1;
}
