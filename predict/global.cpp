#include "global.h"

int debug;
UINT32 curam;
instr ins[INS_SIZE];
UINT8 RAM[RAM_SIZE];
UINT32 regist[CPU_REGISTER_NUMBER];
UINT32 PC, hi, lo;
map<string, UINT32> lbs, ntn;
int time_used;

instr::instr() :type(-1), rdest(-3), rsrc1(-1), rsrc2(-1) {}
instr instr::operator=(const instr &A)
{
	type = A.type, rdest = A.rdest;
	rsrc1 = A.rsrc1, rsrc2 = A.rsrc2;
	imm = A.imm, label = A.label;
	return *this;
}
