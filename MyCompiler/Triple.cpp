#include "Triple.h"
#include "Production.h"
#include <iostream>
#include "Debug.h"
#include "Type.h"
#define NULLADDR -1
Generator* Generator::genPtr = nullptr;
CodeStore* Generator::csPtr = nullptr;
int Generator::codeStart = 0;
vector <string> Generator::icopInt2Str = {
	"+","-","*","/","%","=","id","digit","+=","-=","*=","/=","LD"
};
unordered_map<string, int> Generator::icopStr2Int = {
	{"+",0},{"-",1},{"*",2},{"/",3},{"%",4},\
	{"=", 5}, { "id",6 }, { "digit",7 }, {"+=",8},\
{"-=", 9}, { "*=",10 }, { "/=",11 }, { "LD",12 }
};

Generator::Generator(CodeStore &cs) {
	csPtr = &cs;
}

Generator* Generator::GeneratorFactory(CodeStore &cs) {
	if (genPtr != nullptr)
		return genPtr;
	genPtr = new Generator(cs);
	return genPtr;
}

int Generator::GetIcopInt (string op){
	return icopStr2Int.at(op);
}

int Generator::FindTriple(Triple& t) {
	for (int i = codeStart; i < csPtr->size(); ++i) {
		if ((*csPtr)[i] == t) return i;
	}
	return -1;
}

int Generator::InsertTriple(Triple& t) {
	int find = FindTriple(t);
	if (find != -1) return find;
	else {
		csPtr->push_back(t);
		return csPtr->size() - 1;
	}
}

int Generator::InsertElem(int addr, int val) {
	Triple* temp;
	if (addr == -1) temp = new Triple(ICOP_DIGIT, val);
	else temp = new Triple(ICOP_ID, addr);
	int index = InsertTriple(*temp);
	delete temp;
	return index;
}

int Generator::Gen(int icop,  int code1,int code2) {
	int valNum1 = code1, valNum2 = code2;
#ifdef DEBUG
	ASSERT(valNum1 != -1, "Code : tirple miss");
#endif // DEBUG
	Triple temp(icop, valNum1, valNum2);
	int index = InsertTriple(temp);
	return index;
}

int Generator::GenAssign(int op, int desAddr, int srcCode) {
	Triple temp(op, desAddr, srcCode);
	int index = InsertTriple(temp);
	return index;
}

string Generator::GetIcopStr(int icop) {
	return icopInt2Str.at(icop);
}

static bool IsAssign(Triple& t) {
	if (t.icop == Generator::GetIcopInt("=")) return true;
	else if (t.icop >= Generator::GetIcopInt("+=") && \
		t.icop <= Generator::GetIcopInt("\="))
		return true;
	return false;
}



void Generator::Print() {
	auto& cs = *csPtr;
	int i = 0;
	for (auto& t : cs) {
		cout << i++ << " : " << GetIcopStr(t.icop) + " \t";
		cout << t.valNum1 << "\t" << t.valNum2;
		cout << "\n";
	}
}