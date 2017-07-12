#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

#define INS_SIZE 20000
#define RAM_SIZE 4194304  
#define RAM_UPPER_BOUND 0x003fffff
#define RAM_LOWER_BOUND 0x00000000
#define CPU_REGISTER_NUMBER 32

typedef int INT32;
typedef char INT8;
typedef short INT16;
typedef	unsigned int UINT32;
typedef unsigned char UINT8;
typedef unsigned short UINT16;

struct instr{
	short type, rdest, rsrc1, rsrc2;
	UINT32 imm;
	string label;
	instr();
	instr operator=(const instr &A);
};



#endif
