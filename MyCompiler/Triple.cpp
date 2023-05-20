#include "Triple.h"
#include "Production.h"
#include <iostream>
#include "Debug.h"
#include "Type.h"
#include <iomanip> 
#define NULLADDR -1
Generator* Generator::genPtr = nullptr;
CodeStore* Generator::csPtr = nullptr;
int Generator::codeStart = 0;
stack <list<int>*> Generator::backList;

vector <string> Generator::icopInt2Str = {
	"+","-","*","/","%","=","id","digit","+=","-=","*=","/=","LD","&&",\
	"||","==","!=","<","<=",">",">=","jc","jmp","[]","call","para","ret"
};
unordered_map<string, int> Generator::icopStr2Int = {
	{"+",0},{"-",1},{"*",2},{"/",3},{"%",4},\
	{"=", 5}, { "id",6 }, { "digit",7 }, {"+=",8},\
	{"-=", 9}, { "*=",10 }, { "/=",11 }, { "LD",12 },\
	{"&&", 13}, { "||",14 }, { "==",15 }, {"!=",16},\
	{"<", 17}, { "<=",18 }, { ">",19 }, {">=",20},\
	{"jc", 21}, { "jmp",22 }, { "[]",23 }, {"call",24},\
	{"para",25},{"ret",26}
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

int Generator::BackPatch(int codeIndex, int isOld) {
	if (isOld != 1) backList.push(new list<int>());
	backList.top()->push_back(codeIndex);
	return 1;
}

int Generator::DoPatch(int codeIndex) {
	auto topList = backList.top();
	backList.pop();
	for (auto& i : *topList) {
		(*csPtr)[i].valNum2 = codeIndex;
	}

	delete topList;
	return 1;
}

void Generator::Print() {
	auto& cs = *csPtr;
	int i = 0;
	cout << setiosflags(ios::left);
	for (auto& t : cs) {
		cout << i++ << " : \t"  << setw(15) << GetIcopStr(t.icop);
		if (t.icop == GetIcopInt("id"))
			cout << "0x" << hex << setw(13) <<t.valNum1 << dec << setw(15) <<  t.valNum2;
		else cout << setw(15) <<t.valNum1 << setw(15) << t.valNum2;
		cout << "\n";
	}
}