#include "water.h"

extern int debug;
extern map<string, UINT32> lbs;
extern UINT8 RAM[RAM_SIZE];
extern UINT32 curam;
extern instr ins[INS_SIZE];
extern UINT32 PC, lo, hi, regist[CPU_REGISTER_NUMBER];

int in_use[35];
bool using_memory;

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
			if ((int)ans1 >= RAM_SIZE)
			{
				regist[31] = ans1, in_use[31]--;
//				if (debug)	cerr << "$31 = " << ans1 << ',';
			}
//			if (debug)	cerr << "go to " << ans2 << endl;

			PC = ans2;
			z.state = ppl_work;
		}
//		else if (debug)	cerr << "goto failed" << endl;
	}
	else if (rdest == -1)
	{
		lo = ans1, hi = ans2;
		in_use[33]--, in_use[32]--;
//		if (debug)
	//	{
		//	cerr << "lo = " << lo << ", hi = " << hi << endl;
		//}
	}
	else
	{
		regist[rdest] = ans1, in_use[rdest]--;
		if (debug)	cerr << "regist[" << rdest << "]=" << ans1 << endl;
	}

//	if (debug)
	//{
		//cerr << "";

//	}
}

MA::MA():ans1(1),ans2(0),state(0){}
MA::~MA(){}
void MA::receive(const short&ty, const UINT32&rd,const UINT32&a1, const UINT32&a2)
{
	if (state == ppl_empty)	state = ppl_work;
	type = ty, rdest = rd, ans1 = a1, ans2 = a2;
}
void MA::rstr(const string&fsy)
{
	fsys = fsy;
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
//		if (debug)
	//	{
		//	cerr << "load from memory " << ans2 << "and get " <<ans1 <<',';
		//}
	}
	else if (type == 43)	RAM[ans2] = ans1, using_memory = 1;
	else if (type == 44)	RAM[ans2] = ans1>>8, RAM[ans2+1] = ans1&255, using_memory = 1;
	else if (type == 45)
	{
	//	if (debug)	cerr << endl<<"saving data " << ans1 << "to memory " << ans2 << endl;
		RAM[ans2] = ans1 >> 24, RAM[ans2 + 1] = (ans1 >> 16) & 255;
		RAM[ans2 + 2] = (ans1 >> 8) & 255, RAM[ans2 + 3] = (ans1 & 255), using_memory = 1;
	}
	else if (type == 50)
	{
		cout << ans1;
		if (debug)	cerr << "we are now couting " << ans1 << endl;
	}
	else if (type == 51)
	{
		if (debug)
		{
			cerr << "couting string start from" << ans1 << endl;
			cerr << "they are:";

			int zfsadf = ans1;
			for (char ch = RAM[ans1]; ch != '\0'; ch = RAM[++ans1])	cerr << (int)ch<<":"<<ch<<",   ";
			cerr << endl;
			ans1 = zfsadf;
		}
		for (char ch = RAM[ans1]; ch != '\0'; ch = RAM[++ans1])	cout << ch, using_memory = 1;
	}
	else if (type == 52)
	{
		rdest = 2;
//		if (debug)	cerr << "cin  " << ans1 << endl;
	}
	else if (type == 53)
	{
		using_memory = 1;
		if (debug)	cerr << "cin string" << fsys<<endl;
		int mxl = min((UINT32)fsys.length(),ans2-1);
		for (int i = 0; i < mxl; ++i)	RAM[ans1+i] = fsys[i];
		RAM[ans1+mxl] = '\0';
	}
	else if (type == 54)
	{
//		if (debug)	cerr << "curam from " << curam;
		curam = ans1 + ans2;
//		if (debug)cerr << "to" << curam << endl;
		using_memory = 1;
	}
	else if (type == 55) {
		if (debug)  cerr << "exiting";
		exit(0);
	}
	else if (type == 56) {
		if (debug) 
			cerr << "exiting"; 
		exit(ans2);
	}
	x.receive(rdest, ans1, ans2);
}

EX::EX():ans1(0),ans2(0),state(0){}
EX::~EX(){}
void EX::receive(const UINT32&la, const short&ty, const UINT32&rd, const UINT32&r1, const UINT32&r2)
{
	if (state == ppl_empty)	state = ppl_work;
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
	
//	if (debug)	if (type == 50)	cerr << "syscall, v0 = " << ans1 << ", a0 = " << ans2 << endl;
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
			rdest = -2, ans1 = 1, ans2 = label;	break;
		case 37:										//jal
			rdest = -2, ans1 = rsrc2, ans2 = label; break;
		case 23:{										//beq
			rdest = -2;
			ans1 = (rsrc1==rsrc2);
			ans2 = label;	break;
		}
		case 24:{										//bne
			rdest = -2;
			ans1 = (rsrc1!=rsrc2);
			ans2 = label;	break;
		}
		case 25:{										//bge
			rdest = -2;
			ans1 = ((int)rsrc1>=(int)rsrc2);
			ans2 = label;	break;
		}
		case 26:{										//ble
			rdest = -2;
			ans1 = ((int)rsrc1<=(int)rsrc2);
			ans2 = label;	break;
		}
		case 27:{										//begt
			rdest = -2;
			ans1 = ((int)rsrc1>(int)rsrc2);
			ans2 = label;	break;
		}
		case 28:{										//blt
			rdest = -2;
			ans1 = ((int)rsrc1<(int)rsrc2);
			ans2 = label;	break;
		}
		case 29:{										//beqz
			rdest = -2;
			ans1 = ((int)rsrc1==0);
			ans2 = label;	break;
		}
		case 30:{										//bnez
			rdest = -2;
			ans1 = ((int)rsrc1!=0);
			ans2 = label;	break;
		}
		case 31:{										//blez
			rdest = -2;
			ans1 = ((int)rsrc1<=0);
			ans2 = label;	break;
		}
		case 32:{										//bgez
			rdest = -2;
			ans1 = ((int)rsrc1>=0);
			ans2 = label;	break;
		}
		case 33:{										//bgtz
			rdest = -2;
			ans1 = ((int)rsrc1>0);
			ans2 = label;	break;
		}
		case 34:{										//bltz
			rdest = -2;
			ans1 = ((int)rsrc1<0);
			ans2 = label;	break;
		}
		case 36:										//jr
			rdest = -2, ans1 = 1, ans2 = rsrc1;	break;
		case 38:										//jalr
			rdest = -2, ans1 = rsrc2, ans2 = rsrc1; break;
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
				if (debug)	cerr << "syscall 4 , cout string, rsrc2 = " << rsrc2 << ", rdest = " << rdest << endl;
				type = 51, ans1 = rsrc2, rdest = -3;
			}
			else if (rsrc1 == 5)	type = 52, rdest = 2, in_use[2]++,  cin >> ans1;
			else if (rsrc1 == 8)
			{
				type = 53, ans1 = rsrc2, ans2 = rdest, rdest = -3;
				string z;	cin >> z;	x.rstr(z);
			}
			else if (rsrc1 == 9)	type = 54, rdest = 2, ans2 = rsrc2, in_use[2]++, ans1 = ((curam&3)?(((curam>>2)+1)<<2):curam);
			else if (rsrc1 == 10)	type = 55;
			else if (rsrc1 == 17)	type = 56;
			break;
		}
		default: break;
	}
	if (rdest == -2 && ans1)
	{
		iif.state = ppl_empty, iid.state = ppl_empty;
	}
	x.receive(type,rdest,ans1,ans2);
}


IDDP::IDDP():label(0), type(-1), rdest(-3), rsrc1(-1), rsrc2(-1),state(0){}
IDDP::~IDDP(){}
void IDDP::receive(const instr &x)
{
	if (state == ppl_empty)	state = ppl_work;
	nowins = x;
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


	if (debug)if(type>4 && type<9)	cerr << "type=" << type<<"rdest=" << rdest << endl;
		if ((int)rdest >= 0)
		{
			if (type < 43 || type>45)in_use[rdest]++;
		}
		else if (rdest == -1)
		{
//			if (debug)	cerr << rdest << ' ' << type << endl;
			in_use[32]++, in_use[33]++;
		}
		else if (nowins.rdest == -2)in_use[34]++;
		if (type == 37 || type == 38)in_use[31]++;


		rsrc1 = (nowins.rsrc1 < 0 ? -1 : regist[nowins.rsrc1]);
		rsrc2 = (nowins.rsrc2 < 0 ? nowins.imm : regist[nowins.rsrc2]);

		bool debuguse = 0;


		if (type > 42 && type < 46)
		{
			rdest = regist[rdest];
			
		}

		if (type == 50)	rsrc1 = regist[2], rsrc2 = regist[4], rdest = regist[5];
	z.receive(label, type, rdest, rsrc1, rsrc2);
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
	
	nowins = ins[(PC++)-RAM_SIZE];
	if (nowins.type == 37 || nowins.type == 38)
	{
		nowins.imm = PC;
		
	}
	if (nowins.type == -1)	//read failed
	{
		PC--;
		state = ppl_fail;
		if (z.state == ppl_work)	z.state = ppl_empty;
		return;
	}
	else	z.receive(nowins);
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
			if (debug)while (1);	exit(0);
		}
	}
}
