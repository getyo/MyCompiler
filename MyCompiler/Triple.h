#pragma once
#define ICOP_ADD 0
#define ICOP_SUB 1
#define ICOP_MULT 2
#define ICOP_DIV 3
#define ICOP_REM 4
#define ICOP_ASSIGN 5
#define ICOP_ID 6
#define ICOP_DIG 7
#define ICOP_ASSADD 8
#define ICOP_ASSSUB 9
#define ICOP_ASSMLUT 10
#define ICOP_ASSDIV 11
#define ICOP_LD 12
#define ICOP_AND 13
#define ICOP_OR 14
#define ICOP_EQL 15
#define ICOP_NEQ 16 
#define ICOP_SML 17
#define ICOP_SEQ 18
#define ICOP_BIG 19
#define ICOP_BEQ 20
#define ICOP_JC 21
#define ICOP_JMP 22
#define ICOP_SQUBAC 23
#define ICOP_CALL 24
#define ICOP_PARA 25
#define ICOP_RET 26
#define ICOP_START 27
#define ICOP_END 28
#define ICOP_LABEL 29
#define ICOP_FUN 30

#include <vector>
#include <string>
#include <unordered_map>
#include <list>
#include <stack>
using namespace std;

class Triple{
public:
	//这里的后两个属性，其实是占位用，不一定就是int类型
	int icop;
	int valNum1;
	int valNum2;
	Triple() {}
	Triple(int icop, int valNum1, int valNum2) :\
		icop(icop), valNum1(valNum1), valNum2(valNum2) {};
	Triple(int icop, int val) :icop(icop), valNum1(val) {
		valNum2 = -1;
	};
	Triple(const Triple& t) {
		icop = t.icop;
		valNum1 = t.valNum1;
		valNum2 = t.valNum2;
	}
	bool operator==(const Triple& t) {
		if (icop == t.icop && valNum1 == t.valNum1 && valNum2 == t.valNum2)
			return true;
		return false;
	}
	Triple& operator=(const Triple& t) {
		icop = t.icop;
		valNum1 = t.valNum1;
		valNum2 = t.valNum2;
		return *this;
	}
	Triple& operator=(const Triple&& t) {
		icop = t.icop;
		valNum1 = t.valNum1;
		valNum2 = t.valNum2;
		return *this;
	}
};

typedef vector<Triple> CodeStore;

class Generator {
private:
	static vector<string*> labelSt;
	static int labelSeq;
	static stack<list<int> *> backList;
	static Generator* genPtr;
	static CodeStore* csPtr;
	static vector<string> icopInt2Str;
	static unordered_map<string,int> icopStr2Int;
	Generator(CodeStore&);
	Generator(const Generator& g) {}
	~Generator() {
		list<int>* l;
		while (!backList.empty()) {
			l = backList.top();
			backList.pop();
			delete l;
		}
		for (auto& s : labelSt) delete s;
	}

	static int FindTriple(Triple& t);
	static int InsertTriple(Triple& t);
public:
	static int Label();
	static int codeStart;
	static int GetIcopInt(string op);
	static Generator* GeneratorFactory(CodeStore&);
	static int Gen(int op,int code1,int code2);
	static int InsertElem(int addr, int val);
	static int BackPatch(int codeIndex,int isOld);
	static int DoPatch(int codeIndex);
	static int Update() { 
		codeStart = csPtr->size(); 
		return codeStart; }
	static int GetCodeStart() {
		return codeStart;
	}
	static string GetIcopStr(int icop);
	static void Print();
	static bool ComputeOp(Triple &t);
	static bool AssignOp(Triple& t);
	static void Release() {
		delete genPtr;
	}
};