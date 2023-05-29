#include "Reg.h"
#include "Type.h"
#include "Triple.h"
#include "Debug.h"
#include "Assembly.h"

RegManager* RegManager::regManagerPtr = nullptr;

RegManager* RegManager::RegManagerFactory(CodeStore *cs,AssemblyStore *as) {
	if (regManagerPtr == nullptr)
		regManagerPtr = new RegManager(cs, as);
	return regManagerPtr;
}

bool RegFile::IsLived(int reg) {
	for (auto& v : regStore[reg]) {
		if (v->live) return true;
	}
	return false;
}

bool RegFile::isEmpty(int reg) {
	bool isEmpty = false;
	for (auto& v : regStore[reg]) {
		if (v->reg != REG_EMPTY)
			return false;
	}
	return true;
}

Variable* RegManager::GetOprand(int csSub) {
	if ((*cs)[csSub].icop == ICOP_DIG)
		return nullptr;
	else return (Variable*)((*cs)[csSub].valNum1);
}

string RegManager::StOffset(Variable *v) {
	string s = rf.regName[ REG_EBP ];
	if (v->addr > AssGen::stBase) s += ("-" + to_string(v->addr - AssGen::stBase));
	else if (v->addr < AssGen::stBase) s += ("+" + to_string(AssGen::stBase - v->addr));
	return s;
}

void RegManager::RegSpill(int reg){
	for (auto& v : rf.regStore[reg]) {
		Instruction i;
		i.op = "move";
		i.optype = OpType::ST;

		i.oprand1 = "[" + StOffset(v) + "]";
		i.oprand2 = rf.regName[reg];
		v->reg = REG_EMPTY;

		as->push_back(i);
	}
}

int RegManager::GetBasicReg() {
	for (int i = 0; i < 6; ++i) {
		if (rf.regStore[i].empty())
			return i;
		else if (!rf.IsLived(i))
			return i;
		else if (rf.isEmpty(i))
			return i;
	}
	srand(time(nullptr));
	int reg = rand() % 6;
	RegSpill(reg);
	return reg;
}

int RegManager::LDReg(Instruction &i, Variable* v) {

	if (v->reg != REG_EMPTY) {
		return MULTLD;
	}

	int dst = GetBasicReg();
	rf.regStore[dst].clear();
	rf.regStore[dst].insert(v);
	v->reg = dst;
	i.oprand1 = rf.regName[dst];

	i.oprand2 = "[" + StOffset(v) + "]";
	return 0;
}

int RegManager::STReg(Instruction& i, Triple& ic) {
	Variable* v = GetOprand(ic.valNum1);
#ifdef DEBUG
	ASSERT(v->reg != REG_EMPTY, "Assembly : v isn't in reg");
#endif // DEBUG

	if (v != nullptr) {
		if (v->reg == REG_EMPTY) v->reg = GetBasicReg();
		i.oprand1 = rf.regName[v->reg];
		rf.regStore[v->reg].erase(v);
		v->reg = REG_EMPTY;
	}
	else {
		i.oprand1 = to_string( (*cs)[ic.valNum1].valNum1 );
	}

	return 0;
}

int RegManager::ComputeReg(Instruction& asmi, Triple& ic) {
	Variable* lhs = GetOprand(ic.valNum1);
	Variable* rhs = GetOprand(ic.valNum2);

	if (lhs->reg == REG_EMPTY){
		Instruction ldLhs;
		ldLhs.op = "move";
		ldLhs.optype = OpType::LD;
		LDReg(ldLhs,lhs);
		as->push_back(ldLhs);
	}
	asmi.oprand1 = rf.regName[lhs->reg];
	
	//如果不是常数
	if (rhs != nullptr) {
		if (rhs->reg == REG_EMPTY) {
			Instruction rdLhs;
			rdLhs.op = "move";
			rdLhs.optype = OpType::LD;
			LDReg(rdLhs, rhs);
			as->push_back(rdLhs);
		}
		asmi.oprand2 = rf.regName[rhs->reg];
	}
	else {
		//如果是常数，直接进行计算即可
		asmi.oprand2 = to_string((*cs)[ic.valNum2].valNum1);
	}
	//进行计算后将结果变量保存在ic的valNum1
	ic.valNum1 = (int)lhs;

	return 0;
}

int RegManager::AssignReg(Instruction& asmi, Triple& ic) {
	Variable* dst = GetOprand(ic.valNum1);
	Variable* src = GetOprand(ic.valNum2);
	
	//如果源操作数不是常数
	if (src != nullptr) {
		if (src->reg == REG_EMPTY) {
			Instruction ldSrc;
			ldSrc.op = "move";
			ldSrc.optype = OpType::LD;
			LDReg(ldSrc, src);
			as->push_back(ldSrc);
		}
		rf.regStore[src->reg].insert(dst);
	}
	else {
		int reg = GetBasicReg();
		dst->reg = reg;
		rf.regStore[reg].insert(dst);
	}

	return 0;
}


int RegManager::GetReg(Instruction& asmi, Triple& ic) {
	switch (asmi.optype)
	{
	case OpType::ASSIGN: { return AssignReg(asmi,ic); break; }
	case OpType::COMPUTE: { return ComputeReg(asmi, ic); break; }
	case OpType::LD: {	
		//如果生成汇编直接带有LD指令，说明原来的中间代码是id varaddr的形式
		return LDReg(asmi,(Variable*)ic.valNum1);
		break; }
	case OpType::ST: {
		//如果生成的汇编是st类型，当且仅当实参入栈
		return STReg(asmi, ic);
	}
	default:
		break;
	}
}