#ifndef WATER_H
#define WATER_H

#include "global.h"
#include "predictor.h"

#define ppl_work 0
#define ppl_pause 1
#define ppl_stop 2
#define ppl_empty 3
#define ppl_fail 4

class IF;
class IDDP;
class WB{
	UINT32 rdest, ans1, ans2;
public:
	UINT32 state;	
	WB();
	~WB();
	void receive(const UINT32&rd,const UINT32&a1, const UINT32&a2);
	void run(IF&z);
};
class MA{
	short type;
	UINT32 rdest, ans1, ans2;
	string str_for_syscall;
public:
	short state;
	MA();
	~MA();
	void receive(const short&ty, const UINT32&rd,const UINT32&a1, const UINT32&a2);
	void run(WB &x);
	void getstr(const string &str);
};
class EX{
	short type;
	UINT32 rdest,label, rsrc1, rsrc2;
	UINT32 ans1, ans2, ipc;
public:
	short state;
	EX();
	~EX();
	void receive(const UINT32&la, const short&ty, const UINT32&rd, const UINT32&r1, const UINT32&r2, const UINT32&ip);
	void run(MA &x, IF&iif, IDDP&iid);
};

class IDDP{
	instr nowins;
	short type;
	UINT32 ipc;
	UINT32 rdest, label, rsrc1, rsrc2;
public:
	short state;
	IDDP();
	~IDDP();
	void receive(const instr &x, const UINT32 &ip);
	void run(EX &z);
};

class IF{
	instr nowins;
public:
	short state;
	IF();
	~IF();
	void run(IDDP &z);
};

void next_cycle();
void cpurun();
#endif
