#pragma once
#include <string>
#include <vector>
#include <set>
using namespace std;

class Triple;
struct Variable;
class Environment;

#define REG_EAX 0
#define REG_EBX 1
#define REG_ECX 2
#define REG_EDX 3
#define REG_EDI 4
#define REG_ESI 5
#define REG_EBP 6
#define REG_ESP 7
#define REG_EMPTY -1

#define MULTLD -1;
#define ARRAYLD -2;
enum class OpType	{
	COMPUTE, ASSIGN, LD, ST, OTHER , LABEL
};

struct Instruction {
	OpType optype;
	string op;
	string oprand1;
	string oprand2;
	Instruction(){}
	Instruction(OpType optype,string op,string oprand1 = "",string oprand2 = "")\
		:optype(optype), op(op), oprand1(oprand1), oprand2(oprand2){}
};
typedef vector<Instruction> AssemblyStore;
typedef vector<Triple> CodeStore;
struct RegFile {
	int cnt = 8;
	string regName[8] = {"eax","ebx","ecx","edx","edi","esi","ebp","esp"};
	set<Variable*> regStore[8];

	bool IsLived(int reg);
	bool isEmpty(int reg);
};

class RegManager{
private:
	AssemblyStore* as;
	CodeStore* cs;
	static RegManager* regManagerPtr;
	RegManager(CodeStore *cs,AssemblyStore* as) :as(as),cs(cs) {}

	//返回eax，ebx，ecx，edx,edi,esi中的一个
	int GetBasicReg();

	string ArrayIndex(Variable* a, int index);
	int STReg(Instruction&, Triple&);
	int LDReg(Instruction&, Triple&);
	int LDReg(Instruction&, Variable *);
	int AssignReg(Instruction&, Triple&);
	int ComputeReg(Instruction&, Triple&);
	~RegManager(){}
public:
	RegFile rf;
	
	void RegSpill(int reg);
	Variable* GetOprand(int csSub);
	string GetAddr(Variable * v);
	static RegManager* RegManagerFactory(CodeStore *cs,AssemblyStore * as);
	int GetReg(Instruction&,Triple&);
	static void Release() {
		if (regManagerPtr != nullptr) {
			delete regManagerPtr;
			regManagerPtr = nullptr;
		}
	}
};