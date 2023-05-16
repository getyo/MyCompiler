#include "Type.h"
#include "Parser.h"
#include <iostream>
using namespace std;


int Type::maxDefaultTpyeID = 1;
unordered_set<int> Type::arrayTypeSet;
vector<string> Type::typeID2Str = {
	"int","float"
};
unordered_map<string, int> Type::typeStr2ID = {
	{"int",0}, {"float",1}
};
vector<WidthOrPtr> Type::typeWidth = {
	4,4
};

int Type::GetTypeWidth(int typeID) {
	int a[10];
	if (arrayTypeSet.count(typeID)) {
		auto typePtr = (ArrayType*)typeWidth.at(typeID).typeDef;
		return typePtr->totalSize;
	}
	else return typeWidth.at(typeID).width;
		
}

ArrayType* Type::GetArrayType(int typeID) {
	if (!arrayTypeSet.count(typeID)) return nullptr;
	else return (ArrayType*)typeWidth.at(typeID).typeDef;
}

string Type::CreateArrayStr (int typeID, int dim) {
	string s = GetTypeStr(typeID);
	while (dim-- > 0) s += "[]";
	return s;
}



int Type::CreateArrayType(int typeID, int dim1Size,int dim2Size,int dim3Size) {
	ArrayType* a = new ArrayType(typeID, dim1Size, dim2Size, dim3Size);
	int newID = typeWidth.size();
	string arrStr = CreateArrayStr(a->elemID, a->dim);
	typeWidth.push_back((void*)a);
	typeStr2ID.insert({ arrStr, newID });
	typeID2Str.push_back(arrStr);
	arrayTypeSet.insert(newID);
	return newID;
}


Environmemt* Environmemt::curEnv = nullptr;
size_t Environmemt::dataFieldSize = DATA_START;

static size_t EnvID = 0;

size_t Environmemt::getID() {
	return EnvID++;
}

Environmemt::Environmemt(Environmemt * pre){
	if (pre == nullptr) {
		base = dataFieldSize;
		offset = 0;
		this->pre = nullptr;
		return;
	}
	this->base = dataFieldSize;
	offset = 0;
	this->pre = pre;
}

Environmemt* Environmemt::NewEnv() {
	if (curEnv == nullptr) {
		curEnv = new Environmemt(nullptr);
		return curEnv;
	}
		
	Environmemt* env = new Environmemt(curEnv);
	curEnv = env;
	return env;
}

Environmemt* Environmemt::PopEnv() {
	auto temp = curEnv;
	curEnv = curEnv->pre;
	delete temp;
	return curEnv;
}

bool Environmemt::EnvPush(string lexeme, int typeID) {
	if (symTable.count(lexeme)) {
		cout << "Error : Redefination of identifier : " << lexeme << " " << Parser::RowAndCol() << endl;
		exit(1);
		return false;
	}
	symTable.insert({ lexeme,Variable(typeID,lexeme,offset + base) });
	offset += symTable[lexeme].t.width;
}

Variable& Environmemt::EnvGet(string lexeme) {
	if (!symTable.count(lexeme)) {
		cout << "Error : No identifier defined : " << lexeme <<  " " << Parser::RowAndCol() << endl;
		exit(1);
	}
	return symTable.at(lexeme);
}

ArrayType* Environmemt::GetArrayType(string lex) {
	Type t = curEnv->EnvGet(lex).t;
	auto ap = Type::GetArrayType(t.typeID);
	if (ap == nullptr) {
		cout << "Error : " << lex << " has Type " << Type::GetTypeStr(t.typeID) << \
		" which isn't array type \t" + Parser::RowAndCol() << "\n";
		exit(1);
	}
	return ap;
}

void Environmemt::Print() {
	Environmemt* te = curEnv;
	while (te != nullptr) {
		cout << "Env" << te->envID << ": \n";
		cout << " \tlexeme \ttype \taddr \t\twidth\n";
		for (auto& i : te->symTable) {
			cout << " \t" << i.first << " \t" << Type::GetTypeStr(i.second.t.typeID) << \
			" \t" <<  hex << "0x" << i.second.addr << dec << " \t" << i.second.t.width << "\n";
		}
		te = curEnv->pre;
	}
}

ArrayType::ArrayType(int elemID, int rowSize1, int rowSize2, int rowSize3) :\
elemID(elemID), rowSize1(rowSize1), rowSize2(rowSize2), rowSize3(rowSize3) {
	elemWidth = Type::GetTypeWidth(elemID);
	totalSize = rowSize1 * elemWidth;
	dim = 1;
	if (rowSize2 != ATTR_NON) {
		if (rowSize2 <= 0) {
			cout << "Error : The size of array must be positive value \t" << Parser::RowAndCol();
			exit(1);
		}
		totalSize *= rowSize2;
		dim += 1;
	}
	if (rowSize3 > 0) {
		if (rowSize2 <= 0) {
			cout << "Error : The size of array must be positive value \t" << Parser::RowAndCol();
			exit(1);
		}
		totalSize *= rowSize3;
		dim += 1;
	}
}


int ArrayType::GetDimSize(int dim) {
	switch (dim)
	{
	case 1: {
		if (rowSize2 > 0 && rowSize3 > 0) return rowSize2 * rowSize3;
		else if (rowSize2 > 0) return rowSize2;
		else return -1;
	}
	case 2: {
		if (rowSize3 > 0) return rowSize3;
		else return -1;
	}
	default: {
		cout << "Error : illegal index " << Parser::RowAndCol() << '\n';
		exit(1);
	}
		   break;
	}
}