#ifndef THREAD_H
#define THREAD_H

#include "global.h"
#include <thread>

class ppl{
	int reinuse[34];
	bool using_memory;
	short if_state, id_state, ex_state, ma_state, wb_state;

	int exit0, exit1;

	bool systemworking;
	instr idins;
	
	int extp, exrd, exr1, exr2, exlb;
	
	string sstr;
	int matp, mard, maa1, maa2;
	
	int wbrdest, wbans1, wbans2;
	
public:
	ppl();
	~ppl();
	void if_simu();
	void id_simu();
	void ex_simu();
	void ma_simu();
	void wb_simu();
	int work();
};
#endif
