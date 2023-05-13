#pragma once
#define ICOP_ADD 0
#define ICOP_MINUS 1
#define ICOP_MULT 2
#define ICOP_DIV 3
#define ICOP_REM 4
#define ICOP_ASSIGN 5
#define ICOP_ID 6
#define ICOP_DIGIT 7
#include <vector>
#include <string>
#include <unordered_map>
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
	static Generator* genPtr;
	static CodeStore* csPtr;
	static vector<string> icopInt2Str;
	static unordered_map<string,int> icopStr2Int;
	Generator(CodeStore&);
	Generator(const Generator& g) {}
	~Generator() {}

	static int FindTriple(Triple& t);
	static int InsertTriple(Triple& t);
public:
	static int codeStart;
	static int GetIcopInt(string op);
	static Generator* GeneratorFactory(CodeStore&);
	static int Gen(int op,int code1,int code2);
	static int GenAssign(int op, int desAddr, int srcCode);
	static int InsertElem(int addr, int val);
	static int Update() { return codeStart = csPtr->size() - 1; }
	static string GetIcopStr(int icop);
	static void Print();
};