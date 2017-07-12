#include "predictor.h"

extern int debug;
Predictor::Predictor():allpres(0),corrects(0)
{
	ptp.clear();
	predicts.clear();
	nowpatern.clear();
	
}
Predictor::~Predictor()
{
	nowpatern.clear();
	ptp.clear();
	predicts.clear();
}
bool Predictor::PCexist(const UINT32 &pc)
{
	return (ptp.find(pc)!=ptp.end());
}
bool Predictor::predict(const UINT32&pc)
{
	if (!PCexist(pc))
	{
		ptp[pc] = nowpatern.size();
		bitset<4> ss;
		nowpatern.push_back(ss);
		zz sss;	predicts.push_back(sss);
	}
	return ((predicts[ptp[pc]].patern[nowpatern[ptp[pc]].to_ulong()].to_ulong())&2);
}
void Predictor::uppre(const UINT32&pc, bool jump)
{
	int Pc = ptp[pc], npt = nowpatern[Pc].to_ulong();
	bitset<2> npr = predicts[Pc].patern[npt];
	if (jump)
	{
		if (npr[0] == 0)	npr[0] = 1;
		else if (npr[1] == 0)	npr <<= 1;
	}
	else
	{
		if (npr[0] == 1)	npr[1] = 0;
		else if (npr[1] == 1)	npr >>= 1;
	}
	predicts[Pc].patern[npt] = npr;
}
void Predictor::uppat(const UINT32&pc, bool jump)
{
	bitset<4> tmp = nowpatern[ptp[pc]];
	tmp <<= 1;	tmp[0] = jump;
	nowpatern[ptp[pc]] = tmp;
}
void Predictor::addres(bool succeed)
{
	++allpres;
	if (succeed)	++corrects;
	if (debug)	if (allpres % 2000 == 0)	prisucc();
}
void Predictor::prisucc()
{
	cerr << "there're " << allpres << "predictions in total, and "<< corrects << "times succeed" << endl;
	cerr << "the success rate is " << ((double)corrects/allpres)*100 << "%" << endl;
}
