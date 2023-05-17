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


//创建基本数据类型
void Type::BasicTypeDef() {
	typePtr.reserve(20);
	Type* intTy = new Type(0, 4);
	Type* floatTy = new Type(1, 4);
	typePtr.push_back(intTy);
	typePtr.push_back(floatTy);
}

vector<Type*> Type::typePtr;

int Type::GetTypeWidth(int typeID) {
	return typePtr.at(typeID)->width;
		
}

ArrayType* Type::GetArrayType(int typeID) {
	ArrayType* ap;
	if (ap = dynamic_cast<ArrayType*>(typePtr.at(typeID))){
		return ap;
	}
	else {
		cout << "Error : The array dimension does not match the definition \t" << Parser::RowAndCol() << "\n";
		exit(1);
		return nullptr;
	}
}



int Type::CreateArrayType(int elemID, int size) {
	int typeID = typePtr.size();
	ArrayType* a = new ArrayType(elemID,size,typeID);
	string typeStr = GetTypeStr(elemID) + "[]";
	typeStr2ID.insert({ typeStr,typeID });
	typeID2Str.push_back(typeStr);
	typePtr.push_back(a);
	return typeID;
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
		Type::BasicTypeDef();
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

ArrayType::ArrayType(int elemID, int size,int arrayTypeID) : elemID(elemID),size(size){
	elemWidth = Type::GetTypeWidth(elemID);
	this->typeID = arrayTypeID;
	this->width = size * elemWidth;
}


