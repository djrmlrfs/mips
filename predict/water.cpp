#include "water.h"

extern int debug;
extern map<string, UINT32> lbs;
extern UINT8 RAM[RAM_SIZE];
extern UINT32 curam;
extern instr ins[INS_SIZE];
extern UINT32 PC, lo, hi, regist[CPU_REGISTER_NUMBER];

int in_use[35];
bool using_memory;
Predictor PRE;
WB::WB():ans1(0),ans2(0),state(0){}
WB::~WB(){}
void WB::receive(const UINT32&rd,const UINT32&a1, const UINT32&a2)
{
	if (state == ppl_empty)	state = ppl_work;
	rdest = rd, ans1 = a1, ans2 = a2;
}
void WB::run(IF &z)
{
	if (state == ppl_empty)	return;
	
	if (rdest == -3)	return;
	else if (rdest == -2)
	{
		if (ans1)
		{
			PC = ans2;	z.state = ppl_work;
		}
	}
	else if (rdest == -1)
	{
		lo = ans1, hi = ans2;
		in_use[33]--, in_use[32]--;
	}
	else
	{
		regist[rdest] = ans1, in_use[rdest]--;
	}

}

MA::MA():ans1(1),ans2(0),state(0),str_for_syscall(){}
MA::~MA(){}
void MA::getstr(const string &str)
{
	str_for_syscall = str;
} 
void MA::receive(const short&ty, const UINT32&rd,const UINT32&a1, const UINT32&a2)
{
	if (state == ppl_empty)	state = ppl_work;
	type = ty, rdest = rd, ans1 = a1, ans2 = a2;
}
void MA::run(WB &x)
{
	using_memory = 0;

	if (state == ppl_empty)
	{
		if (x.state == ppl_work)	x.state = ppl_empty;
		return;
	}
	if (x.state==ppl_work || x.state==ppl_empty)	state = ppl_work;
	else
	{
		if (x.state == ppl_work)	x.state = ppl_empty;
		state = ppl_pause;
		return;
	}
	
	
	if (type == 40)	ans1 = RAM[ans2], using_memory = 1;
	else if (type == 41)
	{
		UINT32 aa = RAM[ans2], bb = RAM[ans2 + 1];
		ans1 = (aa << 8)|bb;	using_memory = 1;
	}
	else if (type == 42)
	{
		UINT32 a1 = RAM[ans2], a2 = RAM[ans2 + 1], a3 = RAM[ans2 + 2], a4 = RAM[ans2 + 3];
		ans1 = (a1 << 24)|(a2 << 16)|(a3 << 8)|a4;	using_memory = 1;
	}
	else if (type == 43)	RAM[ans2] = ans1, using_memory = 1;
	else if (type == 44)	RAM[ans2] = ans1>>8, RAM[ans2+1] = ans1&255, using_memory = 1;
	else if (type == 45)
	{
		RAM[ans2] = ans1 >> 24, RAM[ans2 + 1] = (ans1 >> 16) & 255;
		RAM[ans2 + 2] = (ans1 >> 8) & 255, RAM[ans2 + 3] = (ans1 & 255), using_memory = 1;
	}
	else if (type == 50)
	{
		cout << ans1;
	}
	else if (type == 51)
	{

		for (char ch = RAM[ans1]; ch != '\0'; ch = RAM[++ans1])	cout << ch, using_memory = 1;
	}
	else if (type == 52)
	{
		rdest = 2;
	}
	else if (type == 53)
	{
		using_memory = 1;
		int mxl = min((UINT32)str_for_syscall.length(),ans2-1);
		for (int i = 0; i < mxl; ++i)	RAM[ans1+i] = str_for_syscall[i];
		RAM[ans1+mxl] = '\0';
	}
	else if (type == 54)
	{
		curam = ans1 + ans2;
		using_memory = 1;
	}
	else if (type == 55) {
		if (debug)
		{
			PRE.prisucc();
			while (1);
		}
		exit(0);
	}
	else if (type == 56) {
		if (debug)
		{
			PRE.prisucc();
			while (1);
		}
		exit(ans2);
	}
	x.receive(rdest, ans1, ans2);
}

EX::EX():ans1(0),ans2(0),state(0){}
EX::~EX(){}
void EX::receive(const UINT32&la, const short&ty, const UINT32&rd, const UINT32&r1, const UINT32&r2, const UINT32 &ip)
{
	if (state == ppl_empty)	state = ppl_work;
	ipc = ip;
	label = la, type = ty, rdest = rd, rsrc1 = r1, rsrc2 = r2;
}
void EX::run(MA &x, IF&iif, IDDP&iid)
{
	if (state == ppl_empty)
	{
		if (x.state == ppl_work)		x.state = ppl_empty;
		return;
	}
	if (x.state==ppl_work || x.state==ppl_empty)	state = ppl_work;
	else
	{
		if (x.state == ppl_work)	x.state = ppl_empty;
		state = ppl_pause;
		return;
	}
	
	switch (type){
		case 0:	ans1 = (int)rsrc1+(int)rsrc2;	break;	//add
		case 1: case 2:ans1 = rsrc1+rsrc2;		break;	//addu,addiiu
		case 3: ans1 = (int)rsrc1-(int)rsrc2;	break;	//sub
		case 4: ans1 = rsrc1-rsrc2;				break;	//subu
		case 5:{										//mul
			if ((int)rdest < 0)
			{
				long long aa = (int)rsrc1, bb = (int)rsrc2;
				aa *= bb, ans1 = (aa&0xfffffffff), ans2 = (aa>>32);
			}
			else	ans1 = (int)rsrc1*(int)rsrc2;
			break;
		}
		case 6:{										//mulu
			if ((int)rdest < 0)
			{
				unsigned long long aa = rsrc1, bb = rsrc2;
				aa *= bb, ans1 = (aa&0xffffffff), ans2 = (aa>>32);
			}
			else	ans1 = rsrc1*rsrc2;
			break;
		}
		case 7:{										//div
			int aa = rsrc1, bb = rsrc2;
			if ((int)rdest < 0)	ans2 = aa%bb;
			ans1 = aa/bb;	break;
		}
		case 8:{										//divu
			ans1 = rsrc1/rsrc2;
			if ((int)rdest < 0)	ans2 = rsrc1%rsrc2;
			break;
		}
		case 9:	case 10: ans1 = (rsrc1^rsrc2);	break;	//xor, xoru
		case 11: case 12:	ans1 = -rsrc1;		break;	//neg, negu
		case 13: ans1 = (int)rsrc1%(int)rsrc2;	break;	//rem
		case 14: ans1 = rsrc1%rsrc2;			break;	//remu
		case 15: ans1 = rsrc2;					break;	//li
		case 16: ans1 = (rsrc1==rsrc2);			break;	//seq
		case 17: ans1 = ((int)rsrc1>=(int)rsrc2);			break;	//sge
		case 18: ans1 = ((int)rsrc1>(int)rsrc2);			break;	//sgt
		case 19: ans1 = ((int)rsrc1<=(int)rsrc2);			break;	//sle
		case 20: ans1 = ((int)rsrc1<(int)rsrc2);			break; 	//slt
		case 21: ans1 = ((int)rsrc1!=(int)rsrc2);			break; 	//sne
		case 22: case 35:								//b, j
			rdest = -3;	break;
		case 37:										//jal
			rdest = 31, ans1 = rsrc2; break;
		case 23:{										//beq
			ans1 = (rsrc1==rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 24:{										//bne
			ans1 = (rsrc1!=rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 25:{										//bge
			ans1 = ((int)rsrc1>=(int)rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 26:{										//ble
			ans1 = ((int)rsrc1<=(int)rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}

			break;
		}
		case 27:{										//begt
			ans1 = ((int)rsrc1>(int)rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 28:{										//blt
			ans1 = ((int)rsrc1<(int)rsrc2);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 29:{										//beqz
			ans1 = ((int)rsrc1==0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 30:{										//bnez
			ans1 = ((int)rsrc1!=0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 31:{										//blez
			ans1 = ((int)rsrc1<=0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 32:{										//bgez
			ans1 = ((int)rsrc1>=0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 33:{										//bgtz
			ans1 = ((int)rsrc1>0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 34:{										//bltz
			ans1 = ((int)rsrc1<0);
			PRE.uppre(ipc, ans1);
			PRE.uppat(ipc, ans1);
			if (ans1==rdest)	rdest = -3, PRE.addres(1);
			else
			{
				PRE.addres(0);
				rdest = -2, ans1 = 1, ans2 = label;
				iif.state = ppl_empty, iid.state = ppl_empty;
			}
			break;
		}
		case 36:										//jr
			rdest = -3;	break;
		case 38:										//jalr
			rdest = 31, ans1 = rsrc2; break;
		case 39: case 40: case 41: case 42:				//la, lb, lh, lw;
		{
			ans2 = ((int)rsrc1 < 0 ? label : rsrc1 + (int)rsrc2);
			if (type == 39)	ans1 = ans2;

			break;

		}
		case 43: case 44: case 45:						//sb, sh, sw
		{
			ans1 = rdest, ans2 = ((int)rsrc1 < 0 ? label : rsrc1 + (int)rsrc2), rdest = -3;	break;
		}
		case 46: ans1 = rsrc1;	break;					//move
		case 47: ans1 = hi;	break;						//mfhi
		case 48: ans1 = lo;	break;						//mflo
		case 50:{										//syscall
			if (rsrc1 == 1)			ans1 = rsrc2, rdest = -3;
			else if (rsrc1 == 4)
			{
				type = 51, ans1 = rsrc2, rdest = -3;
			}
			else if (rsrc1 == 5)	type = 52, rdest = 2, in_use[2]++,  cin >> ans1;
			else if (rsrc1 == 8)
			{
				string ss;
				cin >> ss;					x.getstr(ss);
				type = 53, ans1 = rsrc2, ans2 = rdest, rdest = -3;
			}
			else if (rsrc1 == 9)	type = 54, rdest = 2, ans2 = rsrc2, in_use[2]++, ans1 = ((curam&3)?(((curam>>2)+1)<<2):curam);
			else if (rsrc1 == 10)	type = 55;
			else if (rsrc1 == 17)	type = 56;
			break;
		}
		default: break;
	}
	x.receive(type,rdest,ans1,ans2);
}


IDDP::IDDP():label(0), type(-1), rdest(-3), rsrc1(-1), rsrc2(-1),state(0){}
IDDP::~IDDP(){}
void IDDP::receive(const instr &x, const UINT32 & ip)
{
	if (state == ppl_empty)	state = ppl_work;
	ipc = ip, nowins = x;
}
void IDDP::run(EX &z)
{
	if (state == ppl_empty)
	{
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	if (z.state == ppl_work || z.state == ppl_empty)	state = ppl_work;
	else
	{
		if (z.state == ppl_work)	z.state = ppl_empty;
		state = ppl_pause;
		return;
	}

	bool zz = 0;
	if (nowins.type == 47 && in_use[32])	zz = 1;
	if (nowins.type == 48 && in_use[33])	zz = 1;
	if (nowins.rsrc1 >= 0 && in_use[nowins.rsrc1])	zz = 1;
	if (nowins.rsrc2 >= 0 && in_use[nowins.rsrc2])	zz = 1;
	if (nowins.type > 38 && nowins.type < 46)
	{
		if (nowins.type > 42)	if (nowins.rdest >= 0 && in_use[nowins.rdest])	zz = 1;
		if (nowins.rsrc1 >= 0 && in_use[nowins.rsrc1])	zz = 1;
	}
	if (nowins.type == 50)
	{
		if (in_use[2])	zz = 1;
		else if (regist[2] != 10 && regist[2] != 5)
			if (in_use[4])	zz = 1;
			else if (regist[2] == 8 && in_use[5])	zz = 1;
	}
	if (zz)
	{
		state = ppl_stop;
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	else	state = ppl_work;

	rdest = nowins.rdest;
	type = nowins.type, label = lbs[nowins.label];

	
	if ((int)rdest >= 0)
	{
		if (type < 43 || type>45)in_use[rdest]++;
	}
	else if (rdest == -1)
	{
		in_use[32]++, in_use[33]++;
	}


	rsrc1 = (nowins.rsrc1 < 0 ? -1 : regist[nowins.rsrc1]);
	rsrc2 = (nowins.rsrc2 < 0 ? nowins.imm : regist[nowins.rsrc2]);


	if (type == 37 || type == 38)	in_use[31]++;
	if (type>42 && type<46)	rdest = regist[rdest];
	else if (type == 22 || type == 35 || type == 37)
	{
		PC = label;	// goto label
	}
	else if (type == 36 || type == 38)
	{
		PC = rsrc1;				// goto rsrc
	}
	else if (type>22 && type<35)	// need predict
	{
		rdest = PRE.predict(ipc);						//rdedst = 1 - > jump = label
		if (rdest)	PC = label, label = ipc+1;			// label = another choice
	}
	else if (type == 50)	rsrc1 = regist[2], rsrc2 = regist[4], rdest = regist[5];
	z.receive(label, type, rdest, rsrc1, rsrc2,ipc);
}


IF::IF():state(0){}
IF::~IF(){}
void IF::run(IDDP &z)
{
	if (state == ppl_empty)
	{
		if (z.state == ppl_work)		z.state = ppl_empty;
		return;
	}
	if (using_memory)
		state = ppl_stop;
	else	state = ppl_work;
	if (state == ppl_stop)
	{
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	if (z.state==ppl_work || z.state==ppl_empty)	state = ppl_work;
	else
	{
		state = ppl_pause;
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	
	nowins = ins[PC-RAM_SIZE];	++PC;
	if (nowins.type == 37 || nowins.type == 38)
	{
		nowins.imm = PC;
	}
	if (nowins.type == -1)	//read failed
	{
		state = ppl_fail;
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	else	z.receive(nowins,PC-1);
}


IF if_simu;
EX ex_simu;
WB wb_simu;
MA mem_simu;
IDDP id_simu;
extern int time_used;

void next_cycle()
{
	++time_used;
	using_memory = 0;
}
void cpurun()
{
	int tmp = 0;
	for (int i = 0; i < 35; ++i)	in_use[i] = 0;
	wb_simu.state = ppl_empty;
	mem_simu.state = ppl_empty;
	ex_simu.state = ppl_empty;
	id_simu.state = ppl_empty;
	while (1)
	{
		next_cycle();
		wb_simu.run(if_simu);
		mem_simu.run(wb_simu);
		ex_simu.run(mem_simu,if_simu,id_simu);
		id_simu.run(ex_simu);
		if_simu.run(id_simu);
		if (if_simu.state == ppl_fail)	tmp++;
		if (tmp > 4) {
			if (debug)
			{
				cerr << "there's no syscall exit";
				while (1);
			}
			exit(0);
		}
	}
}
