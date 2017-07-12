#include "thread.h"
#include <mutex>

#define ppl_work 0
#define ppl_free 1
#define ppl_stop 2
#define ppl_uped 3

extern map<string, UINT32> lbs;
extern UINT8 RAM[RAM_SIZE];
extern UINT32 curam;
extern int debug;
extern instr ins[INS_SIZE];
extern UINT32 PC, lo, hi, regist[CPU_REGISTER_NUMBER];

std::mutex debug_lock, outlock;

ppl::ppl() :if_state(1), id_state(1), ex_state(1), ma_state(1), wb_state(1)
{
	using_memory = 0;
	for (int i = 0; i < 34; ++i)	reinuse[i] = 0;
	exit0 = exit1 = 0;
}
ppl::~ppl() {}
void ppl::if_simu()
{
	instr nowins;
	while (1)
	{
		if (exit0)	return;
		if (if_state == ppl_stop)	if_state = ppl_uped;
		while (if_state!=ppl_free) std::this_thread::yield();
		if_state = ppl_work;
		while (using_memory)std::this_thread::yield();
		nowins = ins[(PC++) - RAM_SIZE];

	//	if (debug){debug_lock.lock();cerr << "PC = " << PC-1-RAM_SIZE << ", type = " << nowins.type << endl;if (PC - 1 - RAM_SIZE == 327)	cerr << ins[327].type << endl;debug_lock.unlock();}
		if (nowins.type == 37 || nowins.type == 38)	nowins.imm = PC;
		if (nowins.type > 22 && nowins.type < 35)
		{
			nowins.rdest = PC;
		}
		if_state = ppl_free;
		while (id_state != ppl_free)std::this_thread::yield();
		if (if_state == ppl_stop)
		{
			nowins.type = 49;
//			if (debug){debug_lock.lock();	cerr << "IF stopped"<< endl;debug_lock.unlock();}
		}
		while (nowins.type == -1)std::this_thread::yield();
		idins = nowins;
		id_state = ppl_uped;
	}
}
	
void ppl::id_simu()
{
	int type, rdest, rsrc1, rsrc2, label;
	while (1)
	{
		if (exit0)	return;	
		while (id_state != ppl_uped)std::this_thread::yield();
		id_state = ppl_work;
		type = idins.type;
		rdest = idins.rdest;
		label = lbs[idins.label];

		if (type == 49)
		{
			rdest = -3;
			//if (debug){debug_lock.lock(); cerr << "we do nothing in this ID" << endl; debug_lock.unlock();}
		}
		else
		{
			if (type == 47)	while (reinuse[32])std::this_thread::yield();	//mflo, mfhi
			else if (type == 48)	while (reinuse[33])std::this_thread::yield();
			else if (type == 50)					//syscall
			{
				while (reinuse[2])std::this_thread::yield();
				if (regist[2] == 8)	while (reinuse[5])std::this_thread::yield();
				if (regist[2]!=10 && regist[2]!=5)	while (reinuse[4])std::this_thread::yield();
			}
			else if (type>38 && type<46)			//sw
			{
				if (type > 42)	if (rdest >= 0)	while (reinuse[rdest])std::this_thread::yield();
				if (idins.rsrc1 >= 0)	while (reinuse[idins.rsrc1])std::this_thread::yield();
			}
			else
			{
				if (idins.rsrc1 >= 0)	while (reinuse[idins.rsrc1])std::this_thread::yield();
				if (idins.rsrc2 >= 0)	while (reinuse[idins.rsrc2])std::this_thread::yield();
			}
			rsrc1 = (idins.rsrc1 < 0 ? -1 : regist[idins.rsrc1]);
			rsrc2 = (idins.rsrc2 < 0 ? idins.imm : regist[idins.rsrc2]);
//			if (debug){debug_lock.lock();cerr << "ID work: type, rdest, rsrc1, rsrc2, label = ";cerr << type << ' ' << rdest << ' ' << rsrc1 << ' ' << rsrc2 << ' ' << label<<endl;debug_lock.unlock();	}
			if (rdest >= 0)
			{
				if (type<23 || (type>34&&type<43) || type>45)
					reinuse[rdest]++;
			}
			else if (rdest == -1)	reinuse[32]++, reinuse[33]++;
			
			if (type == 37 || type == 38)	reinuse[31]++;
			else if (type == 50)
			{
				rsrc1 = regist[2], rsrc2 = regist[4], rdest = regist[5];
				if (rsrc1==5 || rsrc1==9)	reinuse[2] = 1;
			}
			else if (type>42 && type<46)	rdest = regist[rdest];

			if (type > 21 && type < 39)
			{
				while (if_state != ppl_free)std::this_thread::yield();
				if_state = ppl_stop;	//if jump
				//if (debug){debug_lock.lock();cerr << "ID let if stop\n";debug_lock.unlock();}
			}
		}
		
		id_state = ppl_free;

		while (ex_state != ppl_free)std::this_thread::yield();
		
		extp = type, exrd = rdest, exlb = label, exr1 = rsrc1, exr2 = rsrc2;

		ex_state = ppl_uped;
	}
}
	
void ppl::ex_simu()
{
	int type, rdest, ans1, ans2;
	while (1)
	{
		if (exit0)	return;
		while (ex_state != ppl_uped)std::this_thread::yield();
		ex_state = ppl_work;
		type = extp, rdest = exrd;

		switch (type) {
			case 0:{										//add
				ans1 = exr1 + exr2;

				break;
			}
			case 1: case 2:{									//addu,addiiu
				ans1 = (UINT32)exr1 + (UINT32)exr2;
				break;
			}
			case 3:{											//sub
				ans1 = exr1 - exr2;
				break;
			}
			case 4: {										//subu
				ans1 = (UINT32)exr1 - (UINT32)exr2;				
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "sub " << exr1 << " and " << exr2 << ", get " << ans1 << endl;
					debug_lock.unlock();
				}*/
				break;
			}
			case 5:{										//mul
				if (rdest < 0)
				{
					long long aa = exr1, bb = exr2;
					aa *= bb, ans1 = (aa&0xfffffffff), ans2 = (aa>>32);
					/*if (debug)
					{
						debug_lock.lock();
						cerr << "mul " << (int)exr1 << " and " <<(int) exr2 << ",";
						cerr << "ans1 = "<< ans1 << ", ans2 = " << ans2 << endl;
						debug_lock.unlock();
					}*/
				}
				else
				{
					ans1 = exr1*exr2;
					/*if (debug)
					{
						debug_lock.lock();
						cerr << "mul " << exr1 << " and " << exr2 << ", get " << ans1 << endl;
						debug_lock.unlock();
					}*/
				}break;
			}
			case 6:{										//mulu
				if (rdest < 0)
				{
					unsigned long long aa = (UINT32)exr1, bb = (UINT32)exr2;
					aa *= bb, ans1 = (aa&0xffffffff), ans2 = (aa>>32);
					/*if (debug)
					{
						debug_lock.lock();
						cerr << "mul " << exr1 << " and " << exr2 << ",";
						cerr << "ans1 = " << ans1 << ", ans2 = " << ans2 << endl;
						debug_lock.unlock();
					}*/
				}
				else
				{
					ans1 = (UINT32)exr1*(UINT32)exr2;
					/*if (debug)
					{
						debug_lock.lock();
						cerr << "mul " << exr1 << " and " << exr2 << ", get " << ans1 << endl;
						debug_lock.unlock();
					}*/
				}
				break;
			}
			case 7:{										//div
				if (rdest < 0)	ans2 = exr1%exr2;
				ans1 = exr1/exr2;	
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "div, ans1,2 = " <<ans1 << ' ' << ans2 << endl;
					debug_lock.unlock();
				}*/
				break;
			}
			case 8:{										//divu
				UINT32 aa = exr1, bb = exr2;
				if (rdest < 0)	ans2 = aa%bb;
				ans1 = aa/bb;	
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "divu, ans1,2 = " <<ans1 << ' ' << ans2 << endl;
					debug_lock.unlock();
				}*/
				break;
			}
			case 9:	case 10: {								//xor, xoru
				ans1 = (exr1^exr2);
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "xor, ans1 = " <<ans1 << endl;
					debug_lock.unlock();
				}*/
				break;	
			
			}
			case 11: case 12:								//neg, negu
			{
				ans1 = -exr1;
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "neg, ans1 = " <<ans1 << endl;
					debug_lock.unlock();
				}*/
				break;	
			}
			case 13:										//rem
			{	
				ans1 = exr1%exr2;
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "rem, ans1 = " <<ans1 << endl;
					debug_lock.unlock();
				}*/
				break;			
			}	
			case 14: {										//remu
				ans1 = (UINT32)exr1 % (UINT32)exr2; 
				/*if (debug)
				{
					debug_lock.lock();
					cerr << "remu, ans1 = " <<ans1 << endl;
					debug_lock.unlock();
				}
				*/break;
			}
			case 15: {										//li
				ans1 = exr2;
			/*if (debug)
				{
					debug_lock.lock();
					cerr << "li, ans1 = " <<ans1 << endl;
					debug_lock.unlock();
				}
			*/	break;					
			}
			case 16: ans1 = (exr1==exr2);	break;			//seq
			case 17: ans1 = (exr1>=exr2);	break;			//sge
			case 18: ans1 = (exr1>exr2);	break;			//sgt
			case 19: ans1 = (exr1<=exr2);	break;			//sle
			case 20: ans1 = (exr1<exr2);	break;		 	//slt
			case 21: ans1 = (exr1!=exr2);	break; 			//sne
			case 22: case 35:								//b, j
				rdest = -2, ans1 = -1, ans2 = exlb;	break;
			case 37:										//jal
				rdest = -2, ans1 = exr2, ans2 = exlb; break;
			case 23:										//beq
				ans1 = -1, ans2 = (exr1==exr2?exlb:rdest), rdest = -2;	break;
			case 24:										//bne
				ans1 = -1, ans2 = (exr1!=exr2?exlb:rdest), rdest = -2;	break;
			case 25:										//bge
				ans1 = -1, ans2 = (exr1>=exr2?exlb:rdest), rdest = -2;	break;
			case 26:										//ble
				ans1 = -1, ans2 = (exr1<=exr2?exlb:rdest), rdest = -2;	break;
			case 27:										//begt
				ans1 = -1, ans2 = (exr1>exr2?exlb:rdest), rdest = -2;	break;
			case 28:										//blt
				ans1 = -1, ans2 = (exr1<exr2?exlb:rdest), rdest = -2;	break;
			case 29:										//beqz
				ans1 = -1, ans2 = (exr1==0?exlb:rdest), rdest = -2;	break;
			case 30:										//bnez
				ans1 = -1, ans2 = (exr1!=0?exlb:rdest), rdest = -2;	break;
			case 31:										//blez
				ans1 = -1, ans2 = (exr1<=0?exlb:rdest), rdest = -2;	break;
			case 32:										//bgez
				ans1 = -1, ans2 = (exr1>=0?exlb:rdest), rdest = -2;	break;
			case 33:										//bgtz
				ans1 = -1, ans2 = (exr1>0?exlb:rdest), rdest = -2;		break;
			case 34:										//bltz
				ans1 = -1, ans2 = (exr1<0?exlb:rdest), rdest = -2;		break;
			case 36:										//jr
				rdest = -2, ans1 = -1, ans2 = exr1;	break;
			case 38:										//jalr
				rdest = -2, ans1 = exr2, ans2 = exr1;	break;
			case 39: case 40: case 41: case 42:{			//la, lb, lh, lw;
				ans2 = (exr1 < 0 ? exlb : (UINT32)exr1 + exr2);
				if (type == 39)	ans1 = ans2;	break;
			}
			case 43: case 44: case 45:						//sb, sh, sw
				ans1 = rdest, ans2 = (exr1 < 0 ? exlb : (UINT32)exr1+exr2), rdest = -3;	break;
			case 46: ans1 = exr1;	break;					//move
			case 47: ans1 = hi;	break;						//mfhi
			case 48: ans1 = lo;	break;						//mflo
			case 50:{										//syscall
				if (exr1 == 1)			ans1 = exr2, rdest = -3;	//cout int
				else if (exr1 == 4)								//cout string
					type = 51, ans1 = exr2, rdest = -3;
				else if (exr1 == 5)								//cin int
					type = 52, rdest = 2,  cin >> ans1;
				else if (exr1 == 8)
					type = 53, ans1 = exr2, ans2 = rdest, rdest = -3, cin >> sstr;
				else if (exr1 == 9)								//mem
					type = 54, rdest = 2, ans2 = exr2, ans1 = ((curam&3)?(((curam>>2)+1)<<2):curam);
				else if (exr1 == 10)	type = 55;					//exit
				else if (exr1 == 17)	type = 56;					//exit
				break;
			}
			default: break;
		}
		

		
		ex_state = ppl_free;
		while (ma_state != ppl_free)std::this_thread::yield();
		matp = type, mard = rdest, maa1 = ans1, maa2 = ans2;
		ma_state = ppl_uped;
		
	}
}
	
void ppl::ma_simu()
{
	int type, rdest, ans1, ans2;
	while (1)
	{
		if (exit0)	return;
		while (ma_state != ppl_uped )std::this_thread::yield();
		type = matp, rdest = mard, ans1 = maa1, ans2 = maa2;
		ma_state = ppl_work;
		if (type == 40)
		{
			using_memory = 1, ans1 = RAM[ans2];

		//if (debug){debug_lock.lock();cerr << "load date " << ans1 << " from address" << ans2<<endl;debug_lock.unlock();		}
		}
		else if (type == 41)
		{
			using_memory = 1;
			UINT32 aa = RAM[ans2], bb = RAM[ans2 + 1];
			ans1 = (aa << 8) | bb;

			//if (debug){debug_lock.lock();cerr << "load date " << ans1 << " from address" << ans2 << endl;debug_lock.unlock();}
		}
		else if (type == 42)
		{
			using_memory = 1;
			UINT32 a1 = RAM[ans2], a2 = RAM[ans2 + 1], a3 = RAM[ans2 + 2], a4 = RAM[ans2 + 3];
			ans1 = (a1 << 24) | (a2 << 16) | (a3 << 8) | a4;

			//if (debug){debug_lock.lock();cerr << "load date " << ans1 << " from address" << ans2 << endl;debug_lock.unlock();}
		}
		else if (type == 43)
		{
			using_memory = 1, RAM[ans2] = ans1;

//			if (debug){debug_lock.lock();cerr << "save date " << ans1 << " to address" << ans2 << endl;debug_lock.unlock();		}
		}
		else if (type == 44)
		{
			using_memory = 1, RAM[ans2] = ans1 >> 8, RAM[ans2 + 1] = ans1 & 255;

			//if (debug){debug_lock.lock();cerr << "save date " << ans1 << " to address" << ans2 << endl;debug_lock.unlock();	}
		}
		else if (type == 45)
		{
			using_memory = 1;
			RAM[ans2] = ans1 >> 24, RAM[ans2 + 1] = (ans1 >> 16) & 255;
			RAM[ans2 + 2] = (ans1 >> 8) & 255, RAM[ans2 + 3] = (ans1 & 255);

			//if (debug){debug_lock.lock();cerr << "save date " << ans1 << " to address" << ans2 << endl;debug_lock.unlock();	}
		}
		else if (type == 50) {
			outlock.lock();
			cout << ans1;
			outlock.unlock();

			//if (debug){debug_lock.lock();cerr << "cout " << ans1 << endl;debug_lock.unlock();}
		}
		else if (type == 51)
		{
			using_memory = 1;
			outlock.lock();
			for (char ch = RAM[ans1]; ch != '\0'; ch = RAM[++ans1])	cout << ch, using_memory = 1;
			outlock.unlock();
			//if (debug){debug_lock.lock();cerr << "cout a string\n";debug_lock.unlock();}
		}
		else if (type == 52)
		{
			rdest = 2;

			//if (debug){debug_lock.lock();cerr << "cin a int"<<endl;debug_lock.unlock();}
		}
		else if (type == 53)
		{
			using_memory = 1;
			int mxl = min((int)sstr.length(), ans2 - 1);
			for (int i = 0; i < mxl; ++i)	RAM[ans1 + i] = sstr[i];
			RAM[ans1 + mxl] = '\0';

			//if (debug){debug_lock.lock();cerr << "cin a string\n";debug_lock.unlock();}
		}
		else if (type == 54)
		{
			//if (debug){debug_lock.lock();cerr << "curam from " << curam << " to " << ans1 + ans2 << endl;debug_lock.unlock();}
			using_memory = 1, curam = ans1 + ans2;

		}
		else if (type == 55)	{

			if (debug)
			{
				debug_lock.lock();
					cerr << "exit";
				debug_lock.unlock();
			}
			exit1 = 0, exit0 = 1;
			rdest = -3;
		}
		else if (type == 56)
		{
			exit1 = ans2;
			exit0 = 1;
			rdest = -3;
			if (debug)
			{
				debug_lock.lock();
					cerr << "exit " << ans2 << endl;
					cerr << "exit0 = " <<exit0 << endl;
				debug_lock.unlock();
			}
		}

/*		if (debug)
		{
			debug_lock.lock();/*
			if (!using_memory)	{
				cerr << "memory not used\n";
			}/
			debug_lock.unlock();
		}*/
		using_memory = 0;
		ma_state = ppl_free;

		while (wb_state != ppl_free)std::this_thread::yield();
		
		wbrdest = rdest, wbans1 = ans1, wbans2 = ans2;
		wb_state = ppl_uped;
	}
}

void ppl::wb_simu()
{
	int rdest = -3, ans1, ans2;
	while (1)
	{
		if (exit0)	return;
		while (wb_state != ppl_uped)std::this_thread::yield();
		wb_state = ppl_work;

		rdest = wbrdest, ans1 = wbans1, ans2 = wbans2;
		if (rdest == -3)
		{
		}
		else if (rdest == -2)
		{
			if (ans1 >= 0)
			{
				regist[31] = ans1, reinuse[31]--;
			//if (debug){debug_lock.lock();cerr << "$[31] = " << regist[31] << ",";debug_lock.unlock();}
			}
			while (if_state != ppl_uped)std::this_thread::yield();
			PC = ans2;
//			if (debug){debug_lock.lock();cerr << "goto " << ans2 - RAM_SIZE << endl;debug_lock.unlock();}
			if_state = ppl_free;
		}
		else if (rdest == -1)
		{
			lo = ans1, hi = ans2;
			reinuse[32]--, reinuse[33]--;
	//		if (debug){debug_lock.lock();cerr << "lo and hi write " << lo << ' ' << hi << endl;debug_lock.unlock();}
		}
		else
		{
			regist[rdest] = ans1, reinuse[rdest]--;
//			if (debug){debug_lock.lock();cerr << "regist[" << rdest << "] = " << regist[rdest] << endl;debug_lock.unlock();}
		}
		wb_state = ppl_free;
	}
}
	
int ppl::work()
{
	exit0 = 0;
	std::thread WB(&ppl::wb_simu, this);
	std::thread MA(&ppl::ma_simu, this);
	std::thread EX(&ppl::ex_simu, this);
	std::thread ID(&ppl::id_simu, this);
	std::thread IF(&ppl::if_simu, this);
	WB.join();if (debug)cerr << 1;
	MA.join();	if (debug)cerr << 2;
	EX.join();	if (debug)cerr << 3;
	ID.join();	if (debug)cerr << 4;
	IF.join();	if (debug)cerr << 5;
	return exit1;
}