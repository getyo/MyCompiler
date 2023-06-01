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
	auto& t = (*cs)[csSub];
	if (t.icop == ICOP_DIG)
		return nullptr;
	else if (t.icop == ICOP_ID)
		return (Variable*)t.valNum1;
	if (AssGen::beforeGen) return nullptr;
	else return (Variable*)t.valNum1;
}

string RegManager::GetAddr(Variable *v) {
	if (v->e == Environment::Global()) {
		Instruction i;
		i.optype == OpType::OTHER;
		i.op = "lea";
		i.oprand1 = rf.regName[REG_ESI];
		i.oprand2 = "[" + v->name + "]";
		as->push_back(i);
		return rf.regName[REG_ESI];
	}
	string s = rf.regName[ REG_EBP ];
	if (v->belong == nullptr) {
		if (v->addr >= AssGen::stBase) s += ("-" + to_string(v->addr - AssGen::stBase + v->t->width));
		else if (v->addr < AssGen::stBase) s += ("+" + to_string(AssGen::stBase +4 - v->addr));
		return s;
	}
	//数组元素地址
	else {
		//计算数组地址
		if (v->belong->addr >= AssGen::stBase) s += ("-" + to_string(v->belong->addr - AssGen::stBase + v->belong->t->width));
		else if (v->belong->addr < AssGen::stBase) s += ("+" + to_string(AssGen::stBase - v->belong->addr));
		
		//计算下标索引
		Variable* index = GetOprand(-v->addr);
		//索引是常数
		if (index == nullptr) {
			s +=  ("+" + to_string((*cs)[-v->addr].valNum1 * v->t->width));
		}
		else {
			Instruction i;
			i.op = "mov";
			i.optype = OpType::LD;
			if (!LDReg(i, index))
				as->push_back(i);
			s += "+" + rf.regName[index->reg];
		}

		return s;
	}
}

void RegManager::RegSpill(int reg){
	for (auto& v : rf.regStore[reg]) {
		if (!v->live) continue;
		Instruction i;
		i.op = "mov";
		i.optype = OpType::ST;

		i.oprand1 = "[" + GetAddr(v) + "]";
		i.oprand2 = rf.regName[reg];
		v->reg = REG_EMPTY;

		as->push_back(i);
	}
	rf.regStore[reg].clear();
}

int RegManager::GetBasicReg() {
	for (int i = 0; i < 3; ++i) {
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

int RegManager::LDReg(Instruction& i, Triple& t) {
	Variable* v = (Variable*)t.valNum1;
	if (dynamic_cast<ArrayType*>(v->t))
		return ARRAYLD;
	else if (v->addr < 0) {
		return ARRAYLD;
	}
	return LDReg(i, v);
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

	i.oprand2 = "[" + GetAddr(v) + "]";
	return 0;
}

//这里本质上是Push,RegSpill才是真正的Store
int RegManager::STReg(Instruction& i, Triple& ic) {
	Variable* v = GetOprand(ic.valNum1);
#ifdef DEBUG
	if (v->reg == REG_EMPTY) AssGen::assGenPtr->Print();
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

	if (lhs->addr < 0) {
		asmi.oprand1 = GetAddr(lhs);
	}
	else if (lhs->reg == REG_EMPTY){
		Instruction ldLhs;
		ldLhs.op = "mov";
		ldLhs.optype = OpType::LD;
		LDReg(ldLhs,lhs);
		as->push_back(ldLhs);
		asmi.oprand1 = rf.regName[lhs->reg];
	}
	else asmi.oprand1 = rf.regName[lhs->reg];
	
	if (Generator::UnaryOp(ic)) return 0;

	//如果不是常数
	if (rhs != nullptr) {
		if (rhs->addr < 0) {
			asmi.oprand1 = GetAddr(rhs);
		}
		else if (rhs->reg == REG_EMPTY) {
			Instruction rdLhs;
			rdLhs.op = "mov";
			rdLhs.optype = OpType::LD;
			LDReg(rdLhs, rhs);
			as->push_back(rdLhs);
			asmi.oprand2 = rf.regName[rhs->reg];
		}
		else asmi.oprand2 = rf.regName[rhs->reg];
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
	
	if (dst->addr < 0) {
		Instruction i;
		i.op = "mov";
		i.oprand1 = "[" + GetAddr(dst) + "]";

		//如果源操作数不是常数
		if (src != nullptr) {
			if (src->t->IsFunType())
				i.oprand2 = rf.regName[REG_EAX];
			else if (src->reg == REG_EMPTY) {
				Instruction ldSrc;
				ldSrc.op = "mov";
				ldSrc.optype = OpType::LD;
				LDReg(ldSrc, src);
				as->push_back(ldSrc);
			}
			i.oprand2 = rf.regName[src->reg];
		}
		else i.oprand2 = (*cs)[ic.valNum2].valNum1;
		as->push_back(i);
		return 0;
	}

	//如果源操作数不是常数
	if (src != nullptr) {
		if (src->t->IsFunType()) {
			rf.regStore[REG_EAX].insert(dst);
			dst->reg = REG_EAX;
			return 0;
		}
		else if (src->reg == REG_EMPTY) {
			Instruction ldSrc;
			ldSrc.op = "mov";
			ldSrc.optype = OpType::LD;
			LDReg(ldSrc, src);
			as->push_back(ldSrc);
		}
		rf.regStore[src->reg].insert(dst);
		dst->reg = src->reg;
	}
	else {
		Instruction i;
		i.optype = OpType::LD;
		i.op = "mov";
		i.oprand1 = rf.regName[dst->reg];
		i.oprand2 = to_string((*cs)[ic.valNum2].valNum1);
		as->push_back(i);
		return -1;
	}

	return 0;
}


int RegManager::GetReg(Instruction& asmi, Triple& ic) {
	switch (asmi.optype)
	{
	case OpType::ASSIGN: { return AssignReg(asmi,ic); break; }
	case OpType::COMPUTE: { return ComputeReg(asmi, ic); break; }
	case OpType::LD: {	
		return LDReg(asmi,ic);
		break; }
	case OpType::ST: {
		//如果生成的汇编是st类型，当且仅当实参入栈
		return STReg(asmi, ic);
	}
	default:
		break;
	}
}