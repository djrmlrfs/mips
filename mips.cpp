#include "mips.h"

extern int debug;
extern UINT32 curam;
extern instr ins[INS_SIZE];
extern UINT8 RAM[RAM_SIZE];
extern UINT32 regist[CPU_REGISTER_NUMBER];
extern UINT32 PC, hi, lo;
extern map<string, UINT32> lbs, ntn;

#define CRZ curom-RAM_SIZE
int getfromstring(const string &zz)
{
	int x = 0, now = 0, len = zz.length(), f = 1;
	for (; now<len && (zz[now]<'0'||zz[now]>'9'); now++);	if (now>0) if (zz[now-1] == '-')	f = -1;
	for (; now<len && zz[now]>='0' && zz[now]<='9'; now++)	x = x*10+zz[now]-'0';	return x*f;
}
inline bool valid(const char &xx) { return (xx == '_') ||(xx=='#') || (xx >= '0'&&xx <= '9') || (xx >= 'a'&&xx <= 'z') || (xx >= 'A'&&xx <= 'Z'); }
bool simulate_init()
{
	PC = RAM_SIZE;
	ntn["lo"] = 32, ntn["hi"] = 33;
	ntn["zero"] = 0, ntn["at"] = 1, ntn["v0"] = 2, ntn["v1"] = 3;
	ntn["a0"] = 4, ntn["a1"] = 5, ntn["a2"] = 6, ntn["a3"] = 7;
	ntn["t0"] = 8, ntn["t1"] = 9, ntn["t2"] = 10, ntn["t3"] = 11;
	ntn["t4"] = 12, ntn["t5"] = 13, ntn["t6"] = 14, ntn["t7"] = 15;
	ntn["s0"] = 16, ntn["s1"] = 17, ntn["s2"] = 18, ntn["s3"] = 19;
	ntn["s4"] = 20, ntn["s5"] = 21, ntn["s6"] = 22, ntn["s7"] = 23;
	ntn["t8"] = 24, ntn["t9"] = 25, ntn["k0"] = 26, ntn["k1"] = 27;
	ntn["gp"] = 28, ntn["sp"] = 29, ntn["fp"] = 30, ntn["ra"] = 31;
	
	ntn["add"] = 0, ntn["addu"] = 1, ntn["addiu"] = 2, ntn["sub"] = 3;
	ntn["subu"] = 4, ntn["mul"] = 5, ntn["mulu"] = 6, ntn["div"] = 7;
	ntn["divu"] = 8, ntn["xor"] = 9, ntn["xoru"] = 10, ntn["neg"] = 11;
	ntn["negu"] = 12, ntn["rem"] = 13, ntn["remu"] = 14, ntn["li"] = 15;
	ntn["seq"] = 16, ntn["sge"] = 17, ntn["sgt"] = 18, ntn["sle"] = 19, ntn["slt"] = 20, ntn["sne"] = 21;
	ntn["b"] = 22, ntn["beq"] = 23, ntn["bne"] = 24, ntn["bge"] = 25, ntn["ble"] = 26, ntn["bgt"] = 27;
	ntn["blt"] = 28, ntn["beqz"] = 29, ntn["bnez"] = 30, ntn["blez"] = 31, ntn["bgez"] = 32, ntn["bgtz"] = 33;
	ntn["bltz"] = 34, ntn["j"] = 35, ntn["jr"] = 36, ntn["jal"] = 37, ntn["jalr"] = 38;
	ntn["la"] = 39, ntn["lb"] = 40, ntn["lh"] = 41, ntn["lw"] = 42;
	ntn["sb"] = 43, ntn["sh"] = 44, ntn["sw"] = 45, ntn["move"] = 46, ntn["mfhi"] = 47, ntn["mflo"] = 48;
	ntn["nop"] = 49, ntn["syscall"] = 50;
	for (int i = 0; i < 32; ++i)	regist[i] = 0;
	lo = hi = 0;		regist[ntn["sp"]] = RAM_UPPER_BOUND;
	for (int i = 0; i < RAM_SIZE; ++i)	RAM[i] = 0;
	return 1;
}
void get_file(char*na)
{
	int nowmode;
	lbs.clear();
	string name = "";
	debug = 0;
	if (debug)
	{
		name = "array_test1-mahaojun.s";
		freopen("1.in", "r", stdin);
		freopen("1.out", "w", stdout);
	}
	else	name = na;
	ifstream infile;
	infile.open(name.c_str());
	UINT32 curom = RAM_SIZE;
	int lines = 0;
	while (getline(infile,name))
	{
		int len = name.length(), lll = 0;
		while (!valid(name[lll]) && name[lll] != '.' && lll<len)	lll++;
		if (lll >= len)	continue;
		name = name.substr(lll, len - lll);	len -= lll;
		if (name[0] == '#')	continue;
		else if (name[0] == '.')
		{
			if (name[1] == 'a')
				if (name[2] == 'l')
				{
					int n = getfromstring(name.substr(7,len-7));
					if (n > 0)	if (curam&((1<<n)-1))
						curam = ((curam>>(n-1))+1)<<(n-1);
				}
				else
				{
					bool ciiz = 0;
					if (name[6] == 'z')	ciiz = 1, name = name.substr(9, len - 10);
					else	name = name.substr(8, len - 9);
						len = name.length();
					for (int i = 0; i < len; ++i)
						if (name[i] == '\\')
						{
							++i;
							if (name[i] == 'n')	RAM[curam++] = '\n';
							else if (name[i] == 'a')	RAM[curam++] = '\a';
							else if (name[i] == 'b')	RAM[curam++] = '\b';
							else if (name[i] == 'f')	RAM[curam++] = '\f';
							else if (name[i] == 'v')	RAM[curam++] = '\v';
							else if (name[i] == 'r')	RAM[curam++] = '\r';
							else if (name[i] == 't')	RAM[curam++] = '\t';
							else if (name[i] == '\\')	RAM[curam++] = '\\';
							else if (name[i] == '\'')	RAM[curam++] = '\'';
							else if (name[i] == '\"')	RAM[curam++] = '\"';
							else if (name[i] == '\?')	RAM[curam++] = '\?';
							else if (name[i] == '0')	RAM[curam++] = '\0';
						}
						else	RAM[curam++] = name[i];
					if (ciiz)	RAM[curam++] = '\0';
				}
			else if (name[1] == 'b')
			{
				istringstream is(name.substr(6,len-6));
				while (is >> name)
				{
					UINT8 n = getfromstring(name);
					RAM[curam++] = n;
				}
			}
			else if (name[1] == 'h')
			{
				istringstream is(name.substr(6,len-6));
				while (is >> name)
				{
					UINT16 n = getfromstring(name);
					RAM[curam++] = n>>8, RAM[curam++] = n&255;
				}
			}
			else if (name[1] == 'w')
			{
				istringstream is(name.substr(6,len-6));
				while (is >> name)
				{
					UINT32 n = getfromstring(name);
					RAM[curam++] = n>>24, RAM[curam++] = (n>>16)&255;
					RAM[curam++] = (n>>8)&255, RAM[curam++] = n&255;
				}
			}
			else if (name[1] == 's')	curam += getfromstring(name.substr(7,len-7));
			else if (name[1] == 'd')	nowmode = 1;
			else if (name[1] == 't')	nowmode = 0;
		}
		else
		{
			int now = len;
			while (!valid(name[now]))	now--;
			if (name[now+1] == ':')
			{
				int zz = 0;
				while (!valid(name[zz]))	++zz;
				name = name.substr(zz, now - zz + 1);
				lbs[name] = (nowmode==1?curam:curom);
				if (name == "main")	PC = curom;
			}
			else
			{
				string rdest, rsrc1, rsrc2, label;
				istringstream is(name);
				is >> name;
				int work = ntn[name];
				//if (debug)	cerr << "work : " << work << endl;
				ins[CRZ].type = work;
				if (work==0 || work==1 || work==3 || work==4 || work==9 || work==10 || work==13 ||
				work==14 || (work>15&&work<22))
				{
					is >> rdest >> rsrc1 >> rsrc2;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')
						ins[CRZ].rdest = ntn[rdest.substr(1, r - 1)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
					r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r-1)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r-1));
					if (rsrc2[0] != '$')	ins[CRZ].imm = getfromstring(rsrc2);
					else
					{
						r = rsrc2.length()-1;
						if (rsrc2[1]<'0' || rsrc2[1]>'9')
							ins[CRZ].rsrc2 = ntn[rsrc2.substr(1,r)];
						else	ins[CRZ].rsrc2 = getfromstring(rsrc2.substr(1,r));
					}
				}
				else if (work>4 && work<9)
				{
					is >> rdest >> rsrc1;
					if (!(is >> rsrc2))
					{
						int r = rdest.length()-1;
						ins[CRZ].rdest = -1;
						if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rsrc1 = ntn[rdest.substr(1,r-1)];
						else	ins[CRZ].rsrc1 = getfromstring(rdest.substr(1,r-1));
						
						if (rsrc1[0] != '$')	ins[CRZ].imm = getfromstring(rsrc1);
						else
						{
							r = rsrc1.length()-1;
				//			if (debug)	cerr << rsrc1.substr(1, r);
							if (rsrc1[1]<'0' || rsrc1[1]>'9')
							{
						//		if (debug)	cerr << ntn[rsrc1.substr(1, r)] << endl;
								ins[CRZ].rsrc2 = ntn[rsrc1.substr(1, r)];
							}
							else	ins[CRZ].rsrc2 = getfromstring(rsrc1.substr(1,r));
						}
					}
					else
					{
						int r = rdest.length()-1;
						if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r-1)];
						else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
						r = rsrc1.length()-1;
						if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r-1)];
						else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r-1));
						
						if (rsrc2[0] != '$')	ins[CRZ].imm = getfromstring(rsrc2);
						else
						{
							r = rsrc2.length()-1;
							if (rsrc2[1]<'0' || rsrc2[1]>'9')	ins[CRZ].rsrc2 = ntn[rsrc2.substr(1,r)];
							else	ins[CRZ].rsrc2 = getfromstring(rsrc2.substr(1,r));
						}
					}
				}
				else if (work==11 || work==12 || work==46)
				{
					is >> rdest >> rsrc1;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r-1)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
					r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r));
				}
				else if (work == 15)
				{
					is >> rdest >> rsrc1;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r-1)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
					ins[CRZ].imm = getfromstring(rsrc1);
				}
				else if (work == 2)
				{
					is >> rdest >> rsrc1 >> rsrc2;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r-1)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
					r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r-1)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r-1));
					ins[CRZ].imm = getfromstring(rsrc2);
				}
				else if (work==22 || work==35 || work==37)
					is >> ins[CRZ].label;
				else if (work>22 && work<29)
				{
					is >> rsrc1 >> rsrc2;
					int r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r-1)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r-1));
					r = rsrc2.length()-1;
					if (rsrc2[0] != '$')	ins[CRZ].imm = getfromstring(rsrc2.substr(0,r));
					else
					{
						if (rsrc2[1]<'0' || rsrc2[1]>'9')	ins[CRZ].rsrc2 = ntn[rsrc2.substr(1,r-1)];
						else	ins[CRZ].rsrc2 = getfromstring(rsrc2.substr(1,r-1));
					}
					is >> ins[CRZ].label;
				}
				else if (work>28 && work<35)
				{
					is >> rsrc1;
					int r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r-1)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r-1));
					is >> ins[CRZ].label;
				}
				else if (work==36 || work==38)
				{
					is >> rsrc1;
					int r = rsrc1.length()-1;
					if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r)];
					else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r));
				}
				else if (work>38 && work<46)
				{
					is >> rdest >> rsrc1;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r-1)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r-1));
					if ((rsrc1[0]<'0' || rsrc1[0]>'9')&&rsrc1[0]!='-')	ins[CRZ].label = rsrc1;
					else
					{
						ins[CRZ].imm = getfromstring(rsrc1);	r = rsrc1.length() - 1;
						while (rsrc1[r] != ')')	r--;	int l = 0;	while (rsrc1[l] != '(')	l++;
						rsrc1 = rsrc1.substr(l+1,r-1-l);
						r = rsrc1.length()-1;
						if (rsrc1[1]<'0' || rsrc1[1]>'9')	ins[CRZ].rsrc1 = ntn[rsrc1.substr(1,r)];
						else	ins[CRZ].rsrc1 = getfromstring(rsrc1.substr(1,r));
					}
				}
				else if (work==47 || work==48)
				{
					is >> rdest;
					int r = rdest.length()-1;
					if (rdest[1]<'0' || rdest[1]>'9')	ins[CRZ].rdest = ntn[rdest.substr(1,r)];
					else	ins[CRZ].rdest = getfromstring(rdest.substr(1,r));
				}
				if (work>21 && work<39)	ins[CRZ].rdest = -2;
				//if (debug)cerr << work << ' ' << ins[CRZ].rdest << ' ' << ins[CRZ].rsrc1 <<
				//	' ' << ins[CRZ].rsrc2 << ' ' << ins[CRZ].imm<<endl;
				curom++;
			}
			
		}
	}
	infile.close();
	if (debug)	fprintf(stderr, "%d instructions got\n%d memory heap\n", CRZ, curam);

}
int main(int argc, char* argv[])
{
	simulate_init();
	get_file(argv[1]);
	if (debug)	fprintf(stderr, "successfully load file\n");
	if (debug)	cerr << "lbs[_static_0] = " << lbs["_static_0"] << endl;
	cpurun();
}

