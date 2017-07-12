#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "global.h"
#include <bitset>
#include <vector>
using namespace std;
class Predictor{
	vector<bitset<4> >nowpatern;
	map<UINT32,int> ptp;
	struct zz{
		bitset<2> patern[16];
	};
	vector<zz> predicts;

public:
	int allpres, corrects;
	Predictor();
	~Predictor();
	void prisucc();
	bool PCexist(const UINT32 &pc);
	bool predict(const UINT32&pc);
	void uppre(const UINT32&pc, bool jump);
	void uppat(const UINT32&pc, bool jump);
	void addres(bool succeed);
};

#endif
