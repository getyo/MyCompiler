#include "Assembly.h"
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
	i.op = "move";
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

void AssGen::FunEnd() {
	Instruction i;
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

void AssGen::Gen() {
	Instruction i;
	regMan = RegManager::RegManagerFactory(ic, as);

	int lastCmp;
	for (auto& ic : *ic) {
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
			i.op = "move";
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
			i.optype = OpType::OTHER;
			i.op = "call";
			Variable* f = (Variable*)ic.valNum1;
			i.oprand1 = f->name;
			i.oprand2 = "";
			as->push_back(i);
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
			FunEnd();
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
		default:
			break;
		}
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