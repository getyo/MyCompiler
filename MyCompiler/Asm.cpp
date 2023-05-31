#include "Assembly.h"
#include <set>
AssGen* AssGen::assGenPtr = nullptr;
int AssGen::stBase = 0;

AssGen * AssGen::AssGenFactory(AssemblyStore &as){
	if (assGenPtr == nullptr)
		assGenPtr = new AssGen(&as);
	return assGenPtr;
}

void AssGen::SetInput(CodeStore& cs) {
	this->ic = &cs;
}

void AssGen::FunStart(Variable *f) {
	int psize = 0;
	auto fp = dynamic_cast<FunType*>(f->t);
	auto e = f->e;
	for (auto pt : fp->paraType) {
		psize += Type::GetTypeWidth(pt);
	}
	//函数开始开辟栈空间时，使用空间并不计入实参
	int stof = e->size() - psize;
	stBase = psize;
	Instruction i;

	i.optype = OpType::LABEL;
	i.op = f->name + ":";
	as->push_back(i);

	i.optype = OpType::OTHER;
	i.op = "push";
	i.oprand1 = "ebp";
	as->push_back(i);

	i.optype = OpType::ASSIGN;
	i.op = "mov";
	i.oprand1 = "ebp";
	i.oprand2 = "esp";
	as->push_back(i);

	if (stof != 0) {
		i.optype = OpType::COMPUTE;
		i.op = "sub";
		i.oprand1 = "esp";
		i.oprand2 = to_string(stof);
		as->push_back(i);
	}
}

void AssGen::FunEnd(Triple &t) {
	Instruction i;
	if (t.valNum1 > 0) {
		i.op = "mov";
		i.optype = OpType::ASSIGN;
		i.oprand1 = "eax";
		//因为要使用eax，所以把eax所保存的变量全部保存回原地址
		regMan->RegSpill(REG_EAX);

		auto v = regMan->GetOprand(t.valNum1);
		if (v != nullptr) {
			if (!v->t->IsFunType()) {
				if (v->reg == REG_EAX) goto next;
				i.oprand2 = "[" + regMan->GetAddr(v) + "]";
				regMan->rf.regStore[REG_EAX].insert(v);
			}
			else goto next;
		}
		else i.oprand2 = to_string((*ic)[t.valNum1].valNum1);
		as->push_back(i);
	}

	next :
	i.oprand1 = "";
	i.oprand2 = "";
	i.optype = OpType::OTHER;
	i.op = "leave";
	as->push_back(i);

	i.optype = OpType::OTHER;
	i.op = "ret";
	as->push_back(i);
}


void AssGen::GenJc(Triple& t) {
	Instruction i;
	int jop = (*ic)[t.valNum1].icop;
	i.optype = OpType::OTHER;
	switch (jop)
	{
	//在高级语言的逻辑中，应为条件成立不跳转，不成立跳转
	//现在转化为汇编，取高级语言判断条件的补集
	case ICOP_AND: { i.op = "jz"; break; }
	case ICOP_OR: { i.op = "jz"; break; }
	case ICOP_EQL: { i.op = "jz"; break; }
	case ICOP_NEQ: { i.op = "jz"; break; }
	case ICOP_ID: {i.op = "jz"; break; }
	case ICOP_DIG: {i.op = "jz"; break; }
	case ICOP_BEQ: {i.op = "jl"; break; }
	case ICOP_BIG: {i.op = "jle"; break; }
	case ICOP_SML: {i.op = "jge"; break; }
	case ICOP_SEQ: {i.op = "jg"; break; }
	default: {
		i.op = "jz";
	}
		break;
	}

	string* s = (string *)(*ic)[t.valNum2].valNum1;
	i.oprand1 = *s;
	as->push_back(i);
}

void AssGen::GenCall(Triple& t) {
	Instruction i;
	//首先要保存eax的内容，因为要用其保存返回值
	regMan->RegSpill(REG_EAX);

	i.optype = OpType::OTHER;
	i.op = "call";
	Variable* f = (Variable*)t.valNum1;
	i.oprand1 = f->name;
	i.oprand2 = "";
	as->push_back(i);
}

void AssGen::Gen(Block &b) {
	Instruction i;

	int start = b.start;
	for (auto it = ic->begin() + start; start < b.end; ++it, ++start) {
		Triple& ic = *it;
		i.oprand1=  "";
		i.oprand2 = "";
		switch (ic.icop)
		{
		case ICOP_START:{
			FunStart((Variable *)ic.valNum1);
			break;
		}
		case ICOP_ID: {
			i.optype = OpType::LD;
			i.op = "mov";
			if(!regMan->GetReg(i,ic))
				as->push_back(i);
			break;
		}	
		case ICOP_ADD: {
			i.optype = OpType::COMPUTE;
			i.op = "add";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_SUB:{
			i.optype = OpType::COMPUTE;
			i.op = "sub";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_MULT: {
			i.optype = OpType::COMPUTE;
			i.op = "mul";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_DIV: {
			i.optype = OpType::COMPUTE;
			i.op = "div";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_ASSIGN: {
			i.optype = OpType::ASSIGN;
			regMan->GetReg(i, ic);
			break;
		}
		case ICOP_AND: {
			i.optype = OpType::COMPUTE;
			i.op = "and";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_ASSADD: {
			i.optype = OpType::COMPUTE;
			i.op = "add";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_ASSDIV:{
			i.optype = OpType::COMPUTE;
			i.op = "div";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_ASSMLUT : {
			i.optype = OpType::COMPUTE;
			i.op = "mul";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_ASSSUB: {
			i.optype = OpType::COMPUTE;
			i.op = "sub";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_BEQ: {
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_BIG:{
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_CALL: {
			GenCall(ic);
			break;
		}
		case ICOP_EQL: {
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_JC: {
			GenJc(ic);
			break;
		}
		case ICOP_JMP:{
			i.optype = OpType::OTHER;
			i.op = "jmp";
			string* s = (string*)(*this->ic)[ic.valNum1].valNum1;
			i.oprand1 = *s;
			as->push_back(i);
			break;
		}
		case ICOP_NEQ: {
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_OR: {
			i.optype = OpType::COMPUTE;
			i.op = "or";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_PARA: {
			i.optype = OpType::ST;
			i.op = "push";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_RET: {
			FunEnd(ic);
			break;
		}
		case ICOP_SEQ: {
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_SML: {
			i.optype = OpType::COMPUTE;
			i.op = "cmp";
			if (!regMan->GetReg(i, ic))
			as->push_back(i);
			break;
		}
		case ICOP_SQUBAC: {
			
		}
		case ICOP_LABEL: {
			i.optype = OpType::OTHER;
			i.op = *((string*)ic.valNum1) + ":";
			as->push_back(i);
			break;
		}
		case ICOP_POSTINC: {}
		case ICOP_PREINC: {
			i.optype = OpType::COMPUTE;
			i.op = "inc";
			if (!regMan->GetReg(i, ic))
				as->push_back(i);
			break;
		}
		case ICOP_POSTDEC: {}
		case ICOP_PREDEC: {
			i.optype = OpType::COMPUTE;
			i.op = "dec";
			if (!regMan->GetReg(i, ic))
				as->push_back(i);
			break;
		}
		case ICOP_NOT: {
			i.optype = OpType::COMPUTE;
			i.op = "not";
			if (!regMan->GetReg(i, ic))
				as->push_back(i);
			break;
		}
		case ICOP_NEG: {
			i.optype = OpType::COMPUTE;
			i.op = "neg";
			if (!regMan->GetReg(i, ic))
				as->push_back(i);
			break;
		}
		default:
			break;
		}
	}
}

void AssGen::PartBlock() {

	//寻找leader
	CodeStore& cs = *ic;
	int size = cs.size();
	blocks.reserve(100);
	blocks.push_back(Block(0, 0));

	bool followJmp = false;
	vector<int> jmp;
	jmp.push_back(-1);
	for (int i = 0;i< size;i++){
		auto& back = blocks.back();
		if (followJmp) {
			back.end = i;
			jmp.push_back(-1);
			blocks.push_back(Block(i, i));
			followJmp = false;
		}
		switch (cs[i].icop)
		{
		case ICOP_JMP: {
			followJmp = true;
			jmp.back() = cs[i].valNum1;
			break;
		}
		case ICOP_JC: { 
			followJmp = true;
			jmp.back() = cs[i].valNum2;
			break;
		}
		case ICOP_CALL: {
			followJmp = true;
			Variable* f = (Variable*)cs[i].valNum1;
			jmp.back() = f->addr;
			break;
		}
		case ICOP_RET: {
			followJmp = true;
			break;
		}
		default:
			break;
		}
	}
	blocks.back().end = size;

	//构建控制流图
	size = blocks.size();
	for (int i = 0; i < size;++i) {
		if (i + 1 < size)
			blocks[i].next = &blocks[i + 1];
		else
			blocks[i].next = nullptr;
		if (jmp[i] != -1) {
			for (auto& b : blocks) {
				if (b.start == jmp[i])
					blocks[i].jmp = &b;
			}
		}
	}
}

bool AssGen::beforeGen = false;

void AssGen::GlobalData() {
	Instruction i;
	i.op = "[bit32]";
	as->push_back(i);
	i.op = "section .data";
	as->push_back(i);
	

	auto global = Environment::Global();
	auto st = global->GetSymTable();

	for (auto& p : st) {
		Variable* v = p.second;
		if (!v->t->IsFunType()) {
			i.op = v->name + " dd 0";
			as->push_back(i);
		}
	}
}

void AssGen::Gen() {
	GlobalData();
	Instruction i;
	i.op = "section .text";
	as->push_back(i);

	regMan = RegManager::RegManagerFactory(ic, as);
	PartBlock();
	for (auto& b : blocks) {
		beforeGen = true;
		for (int i = b.end - 1; i >= b.start; --i) {
			auto& t = (*ic)[i];
			if (Generator::AssignOp(t) && (*ic)[t.valNum2].icop != ICOP_DIG) {
				Variable* v = regMan->GetOprand(t.valNum1);
				v->live = false;
				v->lastUsed = Variable::UNUSED;
			}
			if (Generator::ComputeOp(t)) {
				Variable* lhs = regMan->GetOprand(t.valNum1);
				Variable* rhs = regMan->GetOprand(t.valNum2);
				if (lhs != nullptr) {
					lhs->live = true;
					lhs->lastUsed = i;
				}
				if (rhs != nullptr) {
					rhs->live = true;
					rhs->lastUsed = i;
				}
			}
		}
		beforeGen = false;
		Gen(b);
	}
}

void AssGen::Print() {
	for (auto& i : *as) {
		if (!i.op.empty()) cout << i.op << " \t";
		else {
			cout << '\n';
			continue;
		}
		if (!i.oprand1.empty()) cout << i.oprand1;
		else {
			cout << '\n';
			continue;
		}
		if (!i.oprand2.empty()) cout << "," << i.oprand2 << " \n";
		else 
			cout << "\n";
	}
}